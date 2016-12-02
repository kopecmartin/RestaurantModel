#include <simlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define CAPACITY 30
#define KITCHEN_SLOTS 6

#define FULL_MENU 0
#define MAIN_COURSE 1
#define SOUP 2

int WAITERS_SIZE;
int KITCH_SUPE_SIZE;       //unused ??

int HOW_MANY_PLATES_WAITER_GET ;
Facility *waiters;




Store restaurant("Store represents maximal capacity of a restaurant", 0);
Store soupKitchen("Store represents how many soups can be done at the same time", 0);
Store mainCourseKitchen("Store represents how many main courses can be prepared at the same time", 0);

Stat waitingForMainCourse("Waiting for a main course");
Stat waitingForSoup("Waiting for a soup");
Stat waitingForPay("Waiting to pay");
Stat waiter1("Counter of waiting processes to be handled by WAITER0");
Stat waiter2("Counter of waiting processes to be handled by WAITER1");
Stat countOfWaitingPlates("Counter of plates");     //never used ?
Stat guestLife("How much time a guest has spent in the restaurant");



Queue waitForSoup;
Queue waitForMainCourse;
Queue orders;
Queue priperdFood;  //unused ??
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
            Wait(Uniform(100,200));
            Leave(soupKitchen);
        } else {
            Enter(mainCourseKitchen);
            counterMainCourses++;
            Wait(Uniform(400,500));
            Leave(mainCourseKitchen);
        }
    }

    void Behavior() {

        kitchen();

        int timeBefore = Time;
        if (isSoup){
            Into(preparedSoups );
        } else {
            Into(preparedMainCourses);
        }
        Passivate();
        Wait(Uniform(10,20));
        Release(waiters[indexActWaiter]);
        printf("Som tu :%d\n", personsProcess.Length());
        while(personsProcess.Length() > 0){
            personsProcess.GetFirst()->Activate();
        }




        // // Impatient food
        // int count = 0;
        // while(true){
        //     count++;
        //     if (isSoup){
        //         if (waitForSoup.Length() > 0){
        //
        //             break;
        //         }

        //     } else {
        //         if (waitForMainCourse.Length() > 0){
        //
        //             break;
        //         }
        //     }
        //     //break;
        //     Wait(1);
        //     printf(" Repeat : %d\n", count );

        // }

    }
};

class Order : public Process {
public:
    int whoWaiter;
    bool manuallyFree;
    int foodCombo;

    Order(int who, int foodCombination) : Process() {
        whoWaiter = who;
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

    void Behavior() {
        //TODO may be interupted

        // printf("New\n");
        int drinksOrdered = 0;

        Seize(waiters[whoWaiter], 1);
        activateFood(foodCombo);

        if (Random() < 0.30){
            drinks++;
            Wait(Uniform(60 , 90 ));
        }
        Wait(Uniform(4 , 6 ));
        Release(waiters[whoWaiter]);

    }
};


class Guest : public Process {
public:

    /**
    * Method represents the selected variant of menu
    * @return Code of variant
    */
    int selectingFood(){

            Wait(Uniform(10, 120));
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

    /**
    * Method represents the conversation between a waiter and a guest.
    * @return Index of waiter who handles a guest process
    */
    void makeOrder(int who, int foodCombo){
        Wait(Uniform(5,10));    // Make conversation with a waiter (creating an order)

        (new Order(who, foodCombo))->Activate();    //Make an order
        Release(waiters[who]);
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


    int indexActWaiter;
    void Behavior() {
        if( !restaurant.Full() ){
            int enterToSystem = Time;
            int foodCombo = selectingFood();


            Enter(restaurant, 1);

            Wait(Uniform(15,40));
            Wait(Uniform(80,150));

            int who = findingFreeWaiter(0);

            Wait(Uniform(15,20));

            makeOrder(who, foodCombo);

            consumingFood(foodCombo);
            int timeBefore = Time;
            who = findingFreeWaiter(1);
            waitingForPay(Time - timeBefore);
            Wait(Uniform(20,60));

            Release(waiters[who]);

            Wait(Uniform(60,100));

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
        waiter1(preparedSoups .Length());
        waiter2(preparedMainCourses.Length());

        Food * main_food;

        if (preparedSoups .Length() > 0 || preparedMainCourses.Length() > 0){
            for (int i = 0; i < WAITERS_SIZE; i++) {
                if (waiters[i].QueueLen() < 4) {
                    int counter = 0;

                    while (counter < HOW_MANY_PLATES_WAITER_GET){
                        //printf(" Indesx %d\n",i );
                        if (preparedSoups .Length() > 0 && waitForSoup.Length() > 0){
                            if(counter == 0){
                                main_food = (Food *)preparedSoups .GetFirst();
                                main_food->indexActWaiter = i;
                                main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                            } else {
                                main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                                preparedSoups .GetFirst()->Cancel();
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



int main(){
    int experimentNumber = 0;
    printf("Enter experiment number: ");
    scanf("%d",&experimentNumber);
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
            printf("Enter capacity of restaurant (defalult value = 30): ");
            scanf("%d",&capacity);

            WAITERS_SIZE = 3;
            HOW_MANY_PLATES_WAITER_GET = 2;

            restaurant.SetCapacity(capacity);
            soupKitchen.SetCapacity(2 * 2);
            mainCourseKitchen.SetCapacity(2 * 3);
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PreparedFoodWatchDog)->Activate();
            Run();
            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 2                      */
        /* node  */
        /* -----------------------------------------------*/
        case 2:
            int countWaiters ;
            printf("Enter count of waiters in the restaurant: ");
            scanf("%d",&countWaiters);

            WAITERS_SIZE = countWaiters;
            HOW_MANY_PLATES_WAITER_GET = 2;

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
        /*              EXPERIMENT 3                      */
        /* node  */
        /* -----------------------------------------------*/
        case 3:
            int countCookers;
            printf("Enter count of cookers in the kitchen: ");
            scanf("%d",&countCookers);

            WAITERS_SIZE = 2;
            HOW_MANY_PLATES_WAITER_GET = 2;

            restaurant.SetCapacity(30);
            soupKitchen.SetCapacity(countCookers * 2);
            mainCourseKitchen.SetCapacity(countCookers * 3);
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PreparedFoodWatchDog)->Activate();
            Run();
            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 4                      */
        /* node  */
        /* -----------------------------------------------*/
        case 4:
            int countPlates;
            printf("Enter count of plates, waiter can carry at the same time: ");
            scanf("%d",&countPlates);

            WAITERS_SIZE = 2;
            HOW_MANY_PLATES_WAITER_GET = countPlates;

            restaurant.SetCapacity(50);
            soupKitchen.SetCapacity(4 * 2);
            mainCourseKitchen.SetCapacity(4 * 3);
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
        case 5:
            printf("5. Experiment starting ... ");




            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 6                      */
        /* node  */
        /* -----------------------------------------------*/
        case 6:
            printf("6. Experiment starting ... ");




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



    countOfWaitingPlates.Output();
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


