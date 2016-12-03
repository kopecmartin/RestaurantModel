#include <simlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#define CAPACITY 30
#define KITCHEN_SLOTS 6

#define FULL_MENU 0
#define MAIN_COURSE 1
#define SOUP 2


int WAITERS_SIZE;


int HOW_MANY_PLATES_WAITER_GET ;
int IMPRUVE_FOOD_QUALITY;
bool DIGITAL_MENU_SYSTEM;

Facility *waiters;




Store restaurant("Store represents maximal capacity of a restaurant", 0);
Store soupKitchen("Store represents how many soups can be done at the same time", 0);
Store mainCourseKitchen("Store represents how many main courses can be prepared at the same time", 0);

Stat waitingForMainCourse("Waiting for a main course");
Stat waitingForSoup("Waiting for a soup");
Stat waitingForPay("Waiting to pay");
Stat waiter1("Counter of waiting processes to be handled by WAITER0");
Stat waiter2("Counter of waiting processes to be handled by WAITER1");

Stat guestLife("How much time a guest has spent in the restaurant");



Queue waitForSoup;
Queue waitForMainCourse;
Queue orders;
Queue peopleWait;
Queue preparedSoups ;
Queue preparedMainCourses;

int counterLeavs = 0;
int counterMainCourses = 0;
int counterSoup = 0;
int drinks = 0;
class Guest;



class Food : public Process {
public:
    bool manuallyActivate;
    bool isSoup;
    int indexActWaiter;
    Queue personsProcess;
    Food(bool soup) : Process() {
        isSoup = soup;
        manuallyActivate = false;
    }

    void kitchen () {
        if (isSoup) {
            Enter(soupKitchen);
            counterSoup++;
            Wait(Uniform(15, 30));
            Leave(soupKitchen);
        } else {
            Enter(mainCourseKitchen);
            counterMainCourses++;
            Wait(Uniform(400 + IMPRUVE_FOOD_QUALITY, 500 + IMPRUVE_FOOD_QUALITY));
            Leave(mainCourseKitchen);
        }
    }

    void Behavior() {

        kitchen();

        if (isSoup){
            Into(preparedSoups );
        } else {
            Into(preparedMainCourses);
        }
        Passivate();
        Wait(Uniform(10,20));
        Release(waiters[indexActWaiter]);
        while(personsProcess.Length() > 0){
            personsProcess.GetFirst()->Activate();
        }
    }
};

class Order : public Process {
public:
    int whoTakeOrder;
    bool manuallyFree;
    int foodCombo;

    Order(int who, int foodCombination) : Process() {
        whoTakeOrder = who;
        manuallyFree = false;
        foodCombo = foodCombination;
    }

    void activateFood(int foodCombo ){
        if (foodCombo == SOUP || foodCombo == FULL_MENU){
            (new Food(true))->Activate();
        }

       if (foodCombo == MAIN_COURSE || foodCombo == FULL_MENU){
            (new Food(false))->Activate();
        }
    }

    int findFreeWaiter() {
        int minQLen = 100;
        int bestWaiter = -1;
        for (int i = 0; i < WAITERS_SIZE; i++) {
            if ((int) waiters[i].QueueLen() < minQLen) {
                bestWaiter = i;
                minQLen = waiters[i].QueueLen();
            }
        }
        return bestWaiter;
    }

    void Behavior() {
        if (DIGITAL_MENU_SYSTEM){
            activateFood(foodCombo);
            if (Random() < 0.40){
                int bestWaiter = findFreeWaiter();
                Seize(waiters[bestWaiter]);
                drinks++;
                Wait(Uniform(60 , 100));
                Release(waiters[bestWaiter]);
            }
        } else {
            Seize(waiters[whoTakeOrder], 1);
            activateFood(foodCombo);
            if (Random() < 0.40){
                drinks++;
                Wait(Uniform(60 , 100));
            }
            Release(waiters[whoTakeOrder]);
        }
    }
};


class Guest : public Process {
public:

    /**
    * Method represents the selected variant of menu
    * @return Code of variant
    */
    int selectingFood(){

            Wait(Uniform(10, 150));
            float randomNum = Random();
            if (randomNum < 0.85){
                return FULL_MENU;
            } else if(randomNum < 0.98){
                return MAIN_COURSE;
            } else {
                return SOUP;
            }
    }

    /**
    * Method will find a free waiter. If all waiters are busy,
    * process will wait for waiter with index 0.
    * @return Index of waiter who handles a guest process
    */
    int findingFreeWaiter(int defaultWaiter){
        int who = -1;
        if (WAITERS_SIZE == 1){
            defaultWaiter = 0;
        }
        if (waiters[defaultWaiter].QueueLen() > 4) {
            for (int i = 1; i < WAITERS_SIZE; i++) {
                if (waiters[i].QueueLen() < 3) {
                    who = i;
                    Seize(waiters[i], 1);
                    break;
                }
            }
        }
        if (who == -1) {
            who = defaultWaiter;
            Seize(waiters[defaultWaiter], 1);
        }
        Wait(Uniform(1,10));        // Waiter is coming to a guest
        return who;
    }


    void consumingFood(int foodCombo){
        if (!(foodCombo == 1)){
            int soupTime = Time;
            Into(waitForSoup);
            Passivate();
            waitingForSoup(Time - soupTime);
            Wait(Uniform(200,400));     //eating soup
        }
        if (!(foodCombo == 2)){
            int soupTime = Time;
            Into(waitForMainCourse);
            Passivate();
            waitingForMainCourse(Time  - soupTime);
            Wait(Uniform(600,1200));        //eating main course
        }

    }

    void Behavior() {
        int who;
        if( !restaurant.Full() ){
            int enterToSystem = Time;
            
            Enter(restaurant, 1);

            Wait(Uniform(15,40));           //time to sit down
            int foodCombo = selectingFood();
            if (DIGITAL_MENU_SYSTEM){
               (new Order(-1, foodCombo))->Activate();
            } else {
                who = findingFreeWaiter(0);
                Wait(Uniform(5,15));    // Make conversation with a waiter (creating an order)
                (new Order(who, foodCombo))->Activate();    //Make an order
                Release(waiters[who]);
            }
            

            consumingFood(foodCombo);
            int timeBefore = Time;
            who = findingFreeWaiter(1);
            waitingForPay(Time - timeBefore);
            Wait(Uniform(20,60));

            Release(waiters[who]);

            if (Random() < 45){
                Wait(Uniform(60,300));
            }

            Wait(Uniform(10,60));

            Leave(restaurant);
            guestLife(Time - enterToSystem);


        } else {
            counterLeavs++;
        }


    }
};

/**
 * Class generates guests in different time intervals
 */
class Generator : public Event {
    void Behavior() {
        (new Guest)->Activate();
        if (Time < 1800){
            Activate(Time + Uniform(0,200));
        } else if(Time < 3600) {
            Activate(Time + Uniform(0,110));
        } else if(Time < 5400) {
            Activate(Time + Uniform(0,80));
        } else if(Time < 7200) {
            Activate(Time + Uniform(0,90));
        } else if(Time < 9000) {
            Activate(Time + Uniform(0,100));
        } else if(Time < 10800) {
            Activate(Time + Uniform(0,180));
        } else if(Time < 12600) {
            Activate(Time + Uniform(0,230));
        } else {
            Activate(Time + Uniform(0,400));
        }
    }
};




class PreparedFoodWatchDog : public Event {

    void Behavior() {
        waiter1(preparedSoups.Length());
        waiter2(preparedMainCourses.Length());

        Food * main_food;

        if (preparedSoups.Length() > 0 || preparedMainCourses.Length() > 0){
            for (int i = 0; i < WAITERS_SIZE; i++) {
                if (waiters[i].QueueLen() < 3) {
                    int counter = 0;
                    while (counter < HOW_MANY_PLATES_WAITER_GET){
                        //printf(" Indesx %d\n",i );
                        if (preparedSoups.Length() > 0 && waitForSoup.Length() > 0){
                            if(counter == 0){
                                main_food = (Food *)preparedSoups.GetFirst();
                                main_food->indexActWaiter = i;
                                main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                            } else {
                                main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                                preparedSoups.GetFirst()->Cancel();
                            }
                            counter++;
                        } else if (preparedMainCourses.Length() > 0 && waitForMainCourse.Length() > 0){
                            if(counter == 0){
                                main_food = (Food *)preparedMainCourses.GetFirst();
                                main_food->indexActWaiter = i;
                                main_food->personsProcess.InsFirst(waitForMainCourse.GetFirst());
                            } else {
                                main_food->personsProcess.InsFirst(waitForMainCourse.GetFirst());
                                preparedMainCourses.GetFirst()->Cancel();
                            }
                            counter++;
                        }  else {
                            break;
                        }
                    }
                    if (counter > 0){
                        waiters[i].QueueIn(main_food, 4);
                    }
                    break;
                }
            }
        }

        Activate(Time + 1);

    }
};

int input(int value){
    string input;
    getline(cin,input);
    if ( !input.empty() ) {
        istringstream stream( input );
        stream >> value;
    }
    return value;
}


int main(){  
     
    int experimentNumber = 0;
    printf("Enter experiment number: ");
    experimentNumber = input(0);
    RandomSeed(time(NULL));
    srand(time(NULL));

    //RandomSeed(Time(NULL));

   
    switch(experimentNumber){
        /* ---------------------------------------------- */
        /*              TEST 0                            */
        /* node  */
        /* -----------------------------------------------*/
        case 0:

            WAITERS_SIZE = 2;
            HOW_MANY_PLATES_WAITER_GET = 2;
            DIGITAL_MENU_SYSTEM = false;
            IMPRUVE_FOOD_QUALITY = 0;

            restaurant.SetCapacity(30);
            soupKitchen.SetCapacity(2 * 2);
            mainCourseKitchen.SetCapacity(2 * 3);
            

            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new PreparedFoodWatchDog)->Activate();
            (new Generator)->Activate();

            Run();
            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 1                      */
        /* node  */
        /* -----------------------------------------------*/
        case 1:
            int capacity ;
            int countWaiters ;
            int countCookers;
            int countPlates;

            printf("Enter capacity of restaurant (defalult value = 30): ");
            capacity = input(30);
            printf("Enter count of waiters in the restaurant (default value = 2): ");
            countWaiters = input(2);
            printf("Enter count of cookers in the kitchen (default value = 2): ");
            countCookers = input(2);
            printf("Enter count of plates, waiter can carry at the same time (default value = 2): ");
            countPlates = input(2);

            WAITERS_SIZE = countWaiters;
            HOW_MANY_PLATES_WAITER_GET = countPlates;
            DIGITAL_MENU_SYSTEM = false;
            IMPRUVE_FOOD_QUALITY = 0;

            restaurant.SetCapacity(capacity);
            soupKitchen.SetCapacity(countCookers * 2);
            mainCourseKitchen.SetCapacity(countCookers * 3);
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PreparedFoodWatchDog)->Activate();
            Run();
            break;


        /* ---------------------------------------------- */
        /*              EXPERIMENT 5                      */
        /* node  */
        /* -----------------------------------------------*/
        case 2:
            WAITERS_SIZE = 2;
            HOW_MANY_PLATES_WAITER_GET = 2;
            DIGITAL_MENU_SYSTEM = true;
            IMPRUVE_FOOD_QUALITY = 0;

            restaurant.SetCapacity(50);
            soupKitchen.SetCapacity(3 * 2);
            mainCourseKitchen.SetCapacity(3 * 3);
            
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PreparedFoodWatchDog)->Activate();
            Run();
            break;

        /* ---------------------------------------------- */
        /*              EXPERIMENT 6                      */
        /* node  */
        /* -----------------------------------------------*/
        case 3:
            WAITERS_SIZE = 2;
            HOW_MANY_PLATES_WAITER_GET = 2;
            DIGITAL_MENU_SYSTEM = true;
            IMPRUVE_FOOD_QUALITY = 60;

            restaurant.SetCapacity(30);
            soupKitchen.SetCapacity(2 * 2);
            mainCourseKitchen.SetCapacity(2 * 3);
            
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PreparedFoodWatchDog)->Activate();
            Run();
            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 7                      */
        /* node  */
        /* -----------------------------------------------*/
        case 7:
            printf("7. Experiment starting ... ");



            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 8                      */
        /* node  */
        /* -----------------------------------------------*/
        case 8:
            printf("8. Experiment starting ... ");
            break;

    }



    waiter1.Output();
    waiter2.Output();
    waitingForSoup.Output();
    waitingForMainCourse.Output();
    waitingForPay.Output();
    guestLife.Output();
    //kitchen.Output();
    printf("Counter of prepared soups: %d\n", counterSoup);
    printf("Counter of prepared main courses: %d\n", counterMainCourses);
    printf("Counter of prepared drinks: %d\n", drinks);
    printf("Leave people:  %d\n", counterLeavs);
    printf("Profit:  %d\n", (counterSoup * 15) + (counterMainCourses * 70) + drinks * 30);



}


