#include <simlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <string>
#include <sstream>

#define CAPACITY 30
#define KITCHEN_SLOTS 6

#define FULL_MENU 0
#define MAIN_COURSE 1
#define SOUP 2

using namespace std;


int WAITERS_SIZE;
int HOW_MANY_PLATES_WAITER_GET;
//true if a new ordering system is used in the restaurant
//it's part of an experiment
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

Queue preparedSoups;
Queue preparedMainCourses;

int counterLeavs = 0;
int drinks = 0;
class Guest;


/**
 * Class represents preparing of food.
 */
class Food : public Process {
public:
    bool isSoup;
    int indexActWaiter;
    Queue personsProcess;

    Food(bool soup) : Process() {
        isSoup = soup;
    }

    void kitchen () {
        if (isSoup) {
            Enter(soupKitchen);
            Wait(Uniform(15, 30));   //time needed to prepare soup
            Leave(soupKitchen);
        } else {
            Enter(mainCourseKitchen);
            Wait(Uniform(400, 500 )); //time needed to prepare main course
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
        Seize(waiters[indexActWaiter], 1);

        Wait(Uniform(10,20));

        Release(waiters[indexActWaiter]);
        while(personsProcess.Length() > 0){
            personsProcess.GetFirst()->Activate();
        }
        personsProcess.Clear();
    }
};


/**
 * Class represents an order
 */
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
                Wait(Uniform(60, 100));
                Release(waiters[bestWaiter]);
            }
        } else {
            Seize(waiters[whoTakeOrder], 1);
            activateFood(foodCombo);
            if (Random() < 0.40){
                drinks++;
                Wait(Uniform(60, 100));
            }
            Release(waiters[whoTakeOrder]);
        }
    }
};


/**
 * Class represents a guest
 */
class Guest : public Process {
public:

    /**
    * Method implements selecting a variant of menu
    * @return Code of menu variant
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

    /**
     * Method implements behaviour of an eating guest
     * @param foodCombo - choosen variant of the menu
     */
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


/**
 * Class is watching prepartion of food (soup and main course),
 * when food is prepared and a guest is waiting for food (soup or main course)
 * class triggers handling of the situation
 */
class PreparedFoodWatchDog : public Event {

    void Behavior() {
        waiter1(preparedSoups.Length());
        waiter2(preparedMainCourses.Length());

        Food * main_food;

        if (preparedSoups.Length() > 0 || preparedMainCourses.Length() > 0){
            int minQLen = waiters[0].QueueLen();
            int noBusyW = 0;

            //find the least busy waiter
            for (int i = 1; i < WAITERS_SIZE; i++) {
                if((int) waiters[i].QueueLen() < minQLen){
                    minQLen = waiters[i].QueueLen();
                    noBusyW = i;
                }
            }

            if (minQLen < 3) {
                int counter = 0;

                //the waiter will get as many plates as he can
                while (counter < HOW_MANY_PLATES_WAITER_GET){
                    if (preparedSoups.Length() > 0 && waitForSoup.Length() > 0){
                        if(counter == 0){
                            main_food = (Food *)preparedSoups.GetFirst();

                            main_food->indexActWaiter = noBusyW;
                            main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                        } else {
                            main_food->personsProcess.InsFirst(waitForSoup.GetFirst());
                            preparedSoups.GetFirst()->Cancel();
                        }
                        counter++;
                    } else if (preparedMainCourses.Length() > 0 && waitForMainCourse.Length() > 0){
                        if(counter == 0){
                            main_food = (Food *)preparedMainCourses.GetFirst();

                            main_food->indexActWaiter = noBusyW;
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
                   main_food->Activate();
                }
            }
        }
        Activate(Time + 1);
    }
};


/**
 * Take input from a user
 * if the user submits no value, use default
 * @param  value - default value
 * @return       - return user's or default value
 */
int input(int value){
    string input;
    getline(cin,input);
    if ( !input.empty() ) {
        istringstream stream( input );
        stream >> value;
    }
    return value;
}


void printStat(){

    waiter1.Output();
    waiter2.Output();
    waitingForSoup.Output();
    waitingForMainCourse.Output();
    waitingForPay.Output();
    guestLife.Output();

    printf("Counter of prepared soups: %lu\n", waitingForSoup.Number());
    printf("Counter of prepared main courses: %lu\n", waitingForMainCourse.Number());
    printf("Counter of prepared drinks: %d\n", drinks);
    printf("Leave people: %d\n", counterLeavs);
    printf("Profit: %lu\n", (waitingForSoup.Number() * 15) + (waitingForMainCourse.Number() * 70) + drinks * 30);
}


void printExperimentDescription(){
    printf("\n--------------Description of experiments:--------------\n");
    printf("Experiment number 0:\n");
    printf("  - Run simulation with real parameters of the restaurant\n\n");
    printf("Experiment number 1:\n");
    printf("  - Run simulation with parameters you choose.\n\n");
    printf("Experiment number 2:\n");
    printf("  - Run simulation with parameters you choose.\n");
    printf("      - enables new ordering system\n\n");
    printf("Experiment number 3:\n");
    printf("  - prints number of lunches after which the investment\n");
    printf("    (new ordering system) would return\n");
    printf("  - only profit higher than profit in experiment 0\n");
    printf("    is taken into account\n\n");
    printf("Experiment number 4:\n");
    printf("  - Run simulation with optional repeats\n");
    printf("      - statistics are more precise then\n\n");
}


/**
 * Experiment calculates number of lunches after which an investment would return.
 * note: Only profit higher than profit in normal state (case 0) is taken into account
 * @param  numberT    number of tables
 * @param  capacity   capacity of the restaurant
 * @param  countCooks number of cooks
 * @param  waiters    number of waiters
 * @param  plates     number of plates waiter can carry at the same time
 */
void investmentReturnsIn(int numberT, int capacity, int countCooks, int waitersNum, int plates){

    int tabletPrice = 1499;
    // price of tablets
    int expenses = tabletPrice * numberT;
    // profit when digital ordering system is not used, case 0
    int profitBefore = 15253;

    printf("\n-----------------SIMULATION------------------\n");
    printf("Number of tablets: %d\nPrice for each: %d\n", numberT, tabletPrice);
    printf("Total investment: %d\n", expenses);
    printf("Original test without tablets, capacity 30, waiters = cooks = plates = 2\n");
    printf("Original profit: %d\n", profitBefore);

    // salary during lunch time = 4 hours
    int waiterSalary = 90 * 4;
    int cookSalary =102 * 4;
    int staffSalary = 0; //sum of staff salary

    int loop = 0;
    int oneLoopProfit = 0;
    double profitAVG = 0;

    WAITERS_SIZE = waitersNum;
    HOW_MANY_PLATES_WAITER_GET = plates;
    DIGITAL_MENU_SYSTEM = true;

    //count day (4 hours = lunch time) staff salary
    //we are gonna take into account only situations when
    //number of staff is different against the normal state (case 0)
    //so that we can find out if hire or fire a staff member
    //is more profitable or not
    if (WAITERS_SIZE == 1) { staffSalary -= waiterSalary; }
    else if (WAITERS_SIZE > 2) {
        staffSalary += waiterSalary * (WAITERS_SIZE - 2);
    }

    if (countCooks == 1) { staffSalary -= cookSalary; }
    else if (countCooks > 2) {
        staffSalary += cookSalary * (countCooks - 2);
    }


    restaurant.SetCapacity(capacity);
    soupKitchen.SetCapacity(countCooks * 2);
    mainCourseKitchen.SetCapacity(countCooks * 3);

    Init(0, 14400);

    while(expenses > 0) {

        waiters = new Facility[WAITERS_SIZE];

        (new Generator)->Activate();
        (new PreparedFoodWatchDog)->Activate();
        Run();

        oneLoopProfit += waitingForSoup.Number() * 15;
        oneLoopProfit += waitingForMainCourse.Number() * 70;
        oneLoopProfit += drinks * 30;

        //one day profit minus staffSalary - profit in normal state
        expenses -= (oneLoopProfit - staffSalary - profitBefore);
        profitAVG += oneLoopProfit;
        oneLoopProfit = 0;
        loop += 1;


        // when statistics are cleared, they are initialized to Time ..
        // therefor before .Clear() is needed to call Init(..)
        Init(0, 14400);
        for (int i = 0; i < WAITERS_SIZE; i++) {
            waiters[i].Clear();
        }
        
        drinks = 0;
        waitForSoup.Clear();
        waitForMainCourse.Clear();
        preparedSoups.Clear();
        preparedMainCourses.Clear();
        restaurant.Clear();
        soupKitchen.Clear();
        mainCourseKitchen.Clear();

        waitingForMainCourse.Clear();
        waitingForSoup.Clear();
        waitingForPay.Clear();
        waiter1.Clear();
        waiter2.Clear();

        guestLife.Clear();
    }

    printf("\n--------------------RESULTS-------------------\n");
    printf("Average profit: %g\n", profitAVG / loop);
    printf("Average profit over original one: %g\n", profitAVG / loop - profitBefore);
    printf("Investment will return in: %d lunch times\n\n", loop);
}


int main(){

    int experimentNumber = 0;

    printExperimentDescription();
    printf("Enter experiment number: ");
    experimentNumber = input(0);

    RandomSeed(time(NULL));
    srand(time(NULL));

    switch(experimentNumber){
        /* ---------------------------------------------- */
        /*                 TEST 0                         */
        /* note: Real parameters of the restaurant        */
        /* -----------------------------------------------*/
        case 0:

            WAITERS_SIZE = 2;
            HOW_MANY_PLATES_WAITER_GET = 2;
            DIGITAL_MENU_SYSTEM = false;

            restaurant.SetCapacity(30);
            soupKitchen.SetCapacity(2 * 2);
            mainCourseKitchen.SetCapacity(2 * 3);

            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new PreparedFoodWatchDog)->Activate();
            (new Generator)->Activate();

            Run();
            printStat();
            break;

        /* -------------------------------------------------------  */
        /*                     EXPERIMENT 1                         */
        /* note: Experiment with restaurant parameters user choose. */
        /* -------------------------------------------------------  */
        case 1:
            int capacity;
            int countWaiters;
            int countCooks;
            int countPlates;

            printf("Enter capacity of the restaurant (defalult value = 30): ");
            capacity = input(30);
            printf("Enter count of waiters in the restaurant (default value = 2): ");
            countWaiters = input(2);
            printf("Enter count of cooks in the kitchen (default value = 2): ");
            countCooks = input(2);
            printf("Enter count of plates, waiter can carry at the same time (default value = 2): ");
            countPlates = input(2);

            WAITERS_SIZE = countWaiters;
            HOW_MANY_PLATES_WAITER_GET = countPlates;
            DIGITAL_MENU_SYSTEM = false;

            restaurant.SetCapacity(capacity);
            soupKitchen.SetCapacity(countCooks * 2);
            mainCourseKitchen.SetCapacity(countCooks * 3);
            waiters = new Facility[WAITERS_SIZE];

            Init(0, 14400);
            (new Generator)->Activate();
            (new PreparedFoodWatchDog)->Activate();
            Run();

            printStat();
            break;

        /* ------------------------------------------------------ */
        /*                      EXPERIMENT 2                      */
        /* note: Experiment with restaurant parameters user choose.*/
        /*       The same as experiment 1 but with electronic     */
        /*       ordering system.                                 */
        /* ------------------------------------------------------ */
        case 2:
            {
                int capacity;
                int countWaiters;
                int countCooks;
                int countPlates;

                printf("Enter capacity of the restaurant (defalult value = 50): ");
                capacity = input(50);
                printf("Enter count of waiters in the restaurant (default value = 3): ");
                countWaiters = input(3);
                printf("Enter count of cooks in the kitchen (default value = 3): ");
                countCooks = input(3);
                printf("Enter count of plates, waiter can carry at the same time (default value = 3): ");
                countPlates = input(3);

                WAITERS_SIZE = countWaiters;
                HOW_MANY_PLATES_WAITER_GET = countPlates;
                DIGITAL_MENU_SYSTEM = true;

                restaurant.SetCapacity(capacity);
                soupKitchen.SetCapacity(countCooks * 2);
                mainCourseKitchen.SetCapacity(countCooks * 3);
                waiters = new Facility[WAITERS_SIZE];

                Init(0, 14400);
                (new Generator)->Activate();
                (new PreparedFoodWatchDog)->Activate();
                Run();

                printStat();
                break;
            }

        /* ----------------------------------------------  */
        /*                EXPERIMENT 3                     */
        /* note: Experiment prints number of lunches after */
        /*       which an investment would return.         */
        /* note: Only profit higher than profit in normal state */
        /*       (case 0) is taken into account            */
        /* ----------------------------------------------- */
        case 3:
            {
                int capacity;
                int countWaiters;
                int countCooks;
                int countPlates;
                int countTablets;

                printf("Enter capacity of the restaurant (defalult value = 30): ");
                capacity = input(30);
                printf("Enter count of waiters in the restaurant (default value = 2): ");
                countWaiters = input(2);
                printf("Enter count of cooks in the kitchen (default value = 2): ");
                countCooks = input(2);
                printf("Enter count of plates, waiter can carry at the same time (default value = 2): ");
                countPlates = input(2);
                printf("Enter count of tablets (default value = 30): ");
                countTablets = input(30);

                investmentReturnsIn(countTablets,
                                    capacity,
                                    countCooks,
                                    countWaiters,
                                    countPlates);
                break;
            }

        /* -------------------------------------------------- */
        /*                  EXPERIMENT 4                      */
        /* note: Experiment allows you to run actually case 2 */
        /*       with optional number of repeats statistics   */
        /*       are more precise then                        */
        /* ---------------------------------------------------*/
        case 4:
        {
            double maxTimeInSystem = 0;
            double minTimeInSystem = 0;
            double avgTimeInSystem = 0;

            // counters of people
            double countOfPeople = 0;
            double countPeopleLeft = 0;

            // counters of soups
            double soupMaxTime = 0;
            double soupMinTime = 0;
            double soupAvgTime = 0;
            double countOfSoup = 0;

            // counters of main courses
            double mcMaxTime = 0;
            double mcMinTime = 0;
            double mcAvgTime = 0;
            double countOfMc = 0;

            double profit = 0;
            double countOfDrinks = 0;


            int capacity;
            int countWaiters;
            int countCooks;
            int countPlates;
            int repeatCount;
            int digitalSystem;  // 1,0 => true, false

            printf("Enter capacity of the restaurant (defalult value = 30): ");
            capacity = input(30);
            printf("Enter count of waiters in the restaurant (default value = 2): ");
            countWaiters = input(2);
            printf("Enter count of cooks in the kitchen (default value = 2): ");
            countCooks = input(2);
            printf("Enter count of plates, waiter can carry at the same time (default value = 2): ");
            countPlates = input(2);
            printf("Enter count of itreation (default value = 20): ");
            repeatCount = input(20);
            printf("Enter 1 if you want to enable a digital ordering system (default = 0): ");
            digitalSystem = input(0);


            WAITERS_SIZE = countWaiters;
            HOW_MANY_PLATES_WAITER_GET = countPlates;
            DIGITAL_MENU_SYSTEM = digitalSystem;

            restaurant.SetCapacity(capacity);
            soupKitchen.SetCapacity(countCooks * 2);
            mainCourseKitchen.SetCapacity(countCooks * 3);


            Init(0, 14400);

            int i;
            for(i = 0; i < repeatCount; i++){

                waiters = new Facility[WAITERS_SIZE];

                (new Generator)->Activate();
                (new PreparedFoodWatchDog)->Activate();
                Run();


                maxTimeInSystem += guestLife.Max();
                minTimeInSystem += guestLife.Min();
                avgTimeInSystem += guestLife.Sum() /  guestLife.Number();
                countOfPeople += guestLife.Number();

                soupMaxTime+= waitingForSoup.Max();
                soupMinTime+= waitingForSoup.Min();
                soupAvgTime+= waitingForSoup.Sum() / waitingForSoup.Number();
                countOfSoup+= waitingForSoup.Number();

                mcMaxTime+= waitingForMainCourse.Max();
                mcMinTime+= waitingForMainCourse.Min();
                mcAvgTime+= waitingForMainCourse.Sum() / waitingForMainCourse.Number();
                countOfMc+= waitingForMainCourse.Number();

                countOfDrinks += drinks;
                countPeopleLeft += counterLeavs;

                profit += waitingForSoup.Number() * 15;
                profit += waitingForMainCourse.Number() * 70;
                profit += drinks * 30;


                drinks = 0;
                counterLeavs = 0;

                // when statistics are cleared, they are initialized to Time ..
                // therefor before .Clear() is needed to call Init(..)
                Init(0, 14400);
                for (int i = 0; i < WAITERS_SIZE; i++) {
                    waiters[i].Clear();
                }

                waitForSoup.Clear();
                waitForMainCourse.Clear();

                preparedSoups.Clear();
                preparedMainCourses.Clear();

                restaurant.Clear();
                soupKitchen.Clear();
                mainCourseKitchen.Clear();

                waitingForMainCourse.Clear();
                waitingForSoup.Clear();
                waitingForPay.Clear();
                waiter1.Clear();
                waiter2.Clear();

                guestLife.Clear();
            }

            printf("\n------------ AVG values OF SYSTEM -------------\n");
            printf("AVG max time in the system %g  (iterations %d)\n", maxTimeInSystem / (i), i);
            printf("AVG min time in the system %g  (iterations %d)\n", minTimeInSystem / (i), i);
            printf("AVG of AVG time in the system %g  (iterations %d)\n", avgTimeInSystem / (i), i);
            printf("AVG count of people in the system %g  (iterations %d)\n", countOfPeople / (i), i);
            printf("AVG people who left %g  (iterations %d)\n", countPeopleLeft / (i), i);
            printf("AVG drinks %g  (iterations %d)\n", countOfDrinks / (i), i);
            printf("AVG profit %g  (iterations %d)\n", profit / (i), i);

            printf("\n------------ AVG values waiting of soup -------------\n");
            printf("AVG max time  %g  (iterations %d)\n", soupMaxTime / (i), i);
            printf("AVG min time  %g  (iterations %d)\n", soupMinTime / (i), i);
            printf("AVG of AVG time  %g  (iterations %d)\n", soupAvgTime / (i), i);
            printf("AVG count of soups  %g  (iterations %d)\n", countOfSoup / (i), i);

            printf("\n------------ AVG values waiting of main course -------------\n");
            printf("AVG max time  %g  (iterations %d)\n", mcMaxTime / (i), i);
            printf("AVG min time  %g  (iterations %d)\n", mcMinTime / (i), i);
            printf("AVG of AVG time  %g  (iterations %d)\n", mcAvgTime / (i), i);
            printf("AVG count of main courses  %g  (iterations %d)\n", countOfMc / (i), i);
            break;
        }
    }
}
