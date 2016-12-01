#include <simlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define CAPACITY 30
#define KITCHEN_SLOTS 6

#define FULL_MENU 0 
#define MAIN_COURSE 1
#define SUPE 2

int WAITERS_SIZE;
int KITCH_SUPE_SIZE;

int HOW_MENY_PLATES_WAITER_GET ;
Facility *waiters;




Store restaurant("Store represented restaurant whit maximal capacity", 0);
Store soupKitchem("Soup kitchen represented how many soups can be done in parallel  ", 0);
Store mainCourseKitchen("Store represented parallel prepar food in kitchen", 0);


Stat waitFotMainCourse("Wait for main course");
Stat waitingForSoup("Wait for sup");
Stat waitingForPay("Wait for pay");
Stat waiter1("Count of waiting processes WAITER0");
Stat waiter2("Count of waiting processes WAITER1");
Stat countOfWaitingPlates("Count of waiting plates");
Stat guestLife("How meny guest restaurant  had");



Queue waitForSoup;
Queue waitForMainCourse;
Queue orders;
Queue priperdFood;
Queue peopleWait;
Queue priperdSups;
Queue priperdMainCourses;

int counterLeavs = 0;
int counterMainCourses = 0;
int counterSoup = 0;
int drinks = 0;
class Guest;



class Food : public Process {
public:
    bool manualyActivate;
    bool isSoup;
    int indexActWaiter;
    Queue personsProcess;
    Food(bool soup) : Process() {
        isSoup = soup;
        manualyActivate = false;
    }

    void kitchen () {
        if (isSoup) {
            Enter(soupKitchem);
            counterSoup++;
            Wait(Uniform(100,200));
            Leave(soupKitchem);
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
            Into(priperdSups);
        } else {
            Into(priperdMainCourses);
        }
        Passivate();
        Wait(Uniform(10,20));
        Release(waiters[indexActWaiter]);
        printf("Somt u :%d\n", personsProcess.Length());
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
    bool mnaualyFree;
    int foodCombo;

    Order(int who, int foodCombination) : Process() {
        whoWaiter = who;
        mnaualyFree = false;
        foodCombo = foodCombination;
    }

    void activateFood(int foodCombo ){
        if (foodCombo == SUPE || foodCombo == FULL_MENU){
            (new Food(true))->Activate();
        }

       if (foodCombo == MAIN_COURSE || foodCombo == FULL_MENU){
            (new Food(false))->Activate();
        }
    }

    void Behavior() {
        //TODO may be interupted
       
        // printf("New\n");
        int drinksOrderd = 0;
        
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
    * Methode reprezented selecting variant of menu
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
                return SUPE;
            }
    }

    /**
    * Methode to find free waiter. If all waiters are busy, process wait for waiter whit index 0.
    * @return Index of waiter who serve guest process
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
        Wait(Uniform(1,10));        // Waiter is coming to guest
        return who;
    }

    /**
    * Methode represented conversation whit waither and creating orders.
    * @return Index of waiter who serve guest process
    */
    void makeOrder(int who, int foodCombo){
        Wait(Uniform(5,10));    // Make conversation with waiter (creating order)
    
        (new Order(who, foodCombo))->Activate();    //Make order
        Release(waiters[who]);
    }

    void consumingFood(int foodCombo){
        if (!(foodCombo == 1)){
            int supeTime = Time;
            Into(waitForSoup);
            Passivate();
            waitingForSoup(Time - supeTime);
            Wait(Uniform(200,400));     //eating soup
        }
        if (!(foodCombo == 2)){
            int supeTime = Time;
            Into(waitForMainCourse);
            Passivate();
            waitFotMainCourse(Time  - supeTime);
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
 * Class to generate guests
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




class PriperdFoodWatchDog : public Event {

    void Behavior() {
        waiter1(priperdSups.Length());
        waiter2(priperdMainCourses.Length());

        Food * main_food;
       
        if (priperdSups.Length() > 0 || priperdMainCourses.Length() > 0){
            for (int i = 0; i < WAITERS_SIZE; i++) {
                if (waiters[i].QueueLen() < 4) {                
                    int counter = 0;
                   
                    while (counter < HOW_MENY_PLATES_WAITER_GET){
                        //printf(" Indesx %d\n",i );
                        if (priperdSups.Length() > 0 && waitForSoup.Length() > 0){
                            if(counter == 0){
                                main_food = (Food *)priperdSups.GetFirst();
                                main_food->indexActWaiter = i;
                                main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                            } else {
                                main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                                priperdSups.GetFirst()->Cancel();   
                            } 
                            counter++;
                        } else if (priperdMainCourses.Length() > 0 && waitForMainCourse.Length() > 0){
                            if(counter == 0){
                                main_food = (Food *)priperdMainCourses.GetFirst();
                                main_food->indexActWaiter = i;
                                main_food->personsProcess.InsFirst(waitForMainCourse.GetFirst());
                            } else {
                                main_food->personsProcess.InsFirst(waitForMainCourse.GetFirst());
                                priperdMainCourses.GetFirst()->Cancel();   
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
    int numbOfExperiment = 0; 
    printf("Enter code of rexperiment : ");
    scanf("%d",&numbOfExperiment);
    RandomSeed(time(NULL));
    srand(time(NULL));
   
    //RandomSeed(Time(NULL));


    switch(numbOfExperiment){
        /* ---------------------------------------------- */
        /*              TEST 0                            */
        /* node  */
        /* -----------------------------------------------*/
        case 0:
           
            WAITERS_SIZE = 2;
            HOW_MENY_PLATES_WAITER_GET = 2;

            restaurant.SetCapacity(30);
            soupKitchem.SetCapacity(2 * 2);
            mainCourseKitchen.SetCapacity(2 * 3);

            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new PriperdFoodWatchDog)->Activate();
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
            HOW_MENY_PLATES_WAITER_GET = 2;
            
            restaurant.SetCapacity(capacity);
            soupKitchem.SetCapacity(2 * 2);
            mainCourseKitchen.SetCapacity(2 * 3);
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PriperdFoodWatchDog)->Activate();
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
            HOW_MENY_PLATES_WAITER_GET = 2;
            
            restaurant.SetCapacity(30);
            soupKitchem.SetCapacity(2 * 2);
            mainCourseKitchen.SetCapacity(2 * 3);
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PriperdFoodWatchDog)->Activate();
            Run();
            break;
           

        /* ---------------------------------------------- */
        /*              EXPERIMENT 3                      */
        /* node  */
        /* -----------------------------------------------*/
        case 3:
            int countCookers;
            printf("Enter count of cookers in the restaurant: ");
            scanf("%d",&countCookers);
            
            WAITERS_SIZE = 2;
            HOW_MENY_PLATES_WAITER_GET = 2;
            
            restaurant.SetCapacity(30);
            soupKitchem.SetCapacity(countCookers * 2);
            mainCourseKitchen.SetCapacity(countCookers * 3);
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PriperdFoodWatchDog)->Activate();
            Run();
            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 4                      */
        /* node  */
        /* -----------------------------------------------*/
        case 4:
            int countPlates;
            printf("Enter count of paltes what waiter get in one time: ");
            scanf("%d",&countPlates);

            WAITERS_SIZE = 2;
            HOW_MENY_PLATES_WAITER_GET = countPlates;

            restaurant.SetCapacity(50);
            soupKitchem.SetCapacity(4 * 2);
            mainCourseKitchen.SetCapacity(4 * 3);
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PriperdFoodWatchDog)->Activate();
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
    waitFotMainCourse.Output();
    waitingForPay.Output();
    guestLife.Output();
    //kitchen.Output();
    printf("Counter of prepared soups: %d\n", counterSoup);
    printf("Counter of prepared main courses: %d\n", counterMainCourses);
    printf("Counter of prepared drinks: %d\n", drinks);
    printf("Leave people:  %d\n", counterLeavs);
    printf("Zisk:  %d\n", (counterSoup * 15) + (counterMainCourses * 70) + drinks * 30);
   


}


