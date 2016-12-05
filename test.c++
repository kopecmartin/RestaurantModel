#include <simlib.h>
#include <stdio.h>
#include <string.h>

#define FULL_MENU 0 
#define MAIN_COURSE 1
#define SUPE 2


int WAITERS_SIZE;
int KITCH_SUPE_SIZE;
int KITCH_MAIN_COUR_SIZE;
Facility *waiters;



Store restaurant("Store represented restaurant whit maximal capacity", 0);
Store soupKitchem("Soup kitchen represented how many soups can be done in parallel  ", 0);
Store mainCourseKitchen("Store represented parallel prepar food in kitchen", 0);

Stat timeIntoSystem("How mutch guests restavrant had");

Stat supeDelay("Stat wait for soup");
Stat mainCourseDelay("Stat wait for main course");

Stat wait1("Waiting to create order");
Stat wait2("Waiting to payment");




Queue waitForSoup;
Queue waitForMainCourse;
Queue orders;
Queue priperdFood;


int counterLeavs = 0;
int counterMainCourses = 0;
int counterSoup = 0;
int drinks = 0;



//Flags that changes system
bool electronicOrderSystem = false;



class Food : public Process {
public:
    bool manualyActivate;
    bool isSoup;
    int indexActWaiter;

    Food(bool soup) : Process() {
        isSoup = soup;
        manualyActivate = false;
    }

    void makeSupe() {
        Enter(soupKitchem);
        counterSoup++;
        Wait(Uniform(60,100));
        Leave(soupKitchem);
    }

    void makeMainCourse() {
        Enter(mainCourseKitchen);
        counterMainCourses++;
        Wait(Uniform(300,500));
        Leave(mainCourseKitchen);
    }

    void Behavior() {
        
        if (isSoup) {
            makeSupe();
        } else {
            makeMainCourse();
        }

        Into(priperdFood);
        Passivate();     
              
        Wait(Uniform(10,20));
           
        Release(waiters[indexActWaiter]);
      
        // Impatient food
        ready:
        if ( isSoup ){
            if (waitForSoup.Length() > 0){
                waitForSoup.GetFirst()->Activate();
                return;
            }
        } else {
            if (waitForMainCourse.Length() > 0){
                waitForMainCourse.GetFirst()->Activate();
                return;
            } 
        }
        Activate(Time + 1);
        Passivate();
        goto ready;
        
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

    void giveOrderToKitchn(int foodCombo){
        if (!(foodCombo == 1)){
            (new Food(true))->Activate();
        }        
        if (!(foodCombo == 2)){
            (new Food(false))->Activate();
        }
    }

    void Behavior() {

        //TODO may be interupted
        printf("New %f\n",Time);
       // Seize(waiters[whoWaiter], 1);
        //   printf("New %f\n",Time);
        // int drinksOrderd = 0;
        // if (!mnaualyFree){
        //     int orderCounter = 1;
        //     if (whoWaiter == 0 && waiters[0].Q1->Length() > 0){
        //         // printf("I have more orders\n");
        //         for(int i = 0; i < (int) waiters[0].Q1->Length(); i++){
        //             orderCounter++;
        //             Order * ord = (Order *)waiters[0].Q1->GetLast();
        //             ord->mnaualyFree = true;
        //             ord->Activate();
        //             if (Random() < 0.30)
        //                 drinksOrderd++;

        Wait(10);
        //         }
        //     }
        //     if (!(foodCombo == 1)){
        //         (new Food(true))->Activate();
        //     }

        //     if (!(foodCombo == 2)){
        //         (new Food(false))->Activate();
        //     }
        //     //pripering drinks
        //     if (Random() < 0.30)
        //         drinksOrderd++;
        //     if (drinksOrderd > 0){
        //         drinks += drinksOrderd;
        //         Wait(Uniform(60 * drinksOrderd, 90 * drinksOrderd));
        //     }

        //     // printf("Waiting mulyiplay %d\n", orderCounter);
        //     Wait(Uniform(4 * orderCounter, 6 * orderCounter));
        //    Release(waiters[whoWaiter]);
        // } else {
        //     if (!(foodCombo == 1)){
        //         (new Food(true))->Activate();
        //     }
            
        //     if (!(foodCombo == 2)){
        //         (new Food(false))->Activate();
        //     }
        // }
    
        // printf("After Seize get order: %d\n", waiters[whoWaiter].Q1->Length());
    }
};


class Guest : public Process {

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
    int findingFreeWaiter(){
        int who = -1;
        if (waiters[0].Busy()) {
            for (int i = 1; i < WAITERS_SIZE; i++) {
                if (!waiters[i].Busy()) {
                    who = i;
                    Seize(waiters[i]);
                    break;
                }
            }
        }
        if (who == -1) {
            who = 0;
            Seize(waiters[0], 3);
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

    void eating(int foodCombo){
        if (foodCombo == SUPE || foodCombo == FULL_MENU){
            Into(waitForSoup);
            Passivate();                //waiting to supe
            Wait(Uniform(280,400));     //eating soup
        }

        if (foodCombo == MAIN_COURSE || foodCombo == FULL_MENU){
            Into(waitForMainCourse);
            Passivate();                //waiting to main course
            Wait(Uniform(280,400));     //eating main course
        }
    }

    void makePaymant(){

        int findedWaiter = findingFreeWaiter();
      
        Wait(Uniform(20,120));           //waiting fot bill
        Release(waiters[findedWaiter]);
    }

   
    
    void Behavior() {
        if( !restaurant.Full() ){
            int stopTime;

            double tvstup = Time;
            Enter(restaurant, 1);

            Wait(Uniform(30, 100));   
            int selectedFoodCombo = selectingFood();

            stopTime = Time;
            int findedWaiter = findingFreeWaiter();
            wait1(Time - stopTime);
            printf("Robim Objednavku \n");
            makeOrder(findedWaiter, selectedFoodCombo);
            Release(waiters[findedWaiter]);
            printf("Papam\n");
            eating(selectedFoodCombo);

            makePaymant();
           
            Wait(Uniform(0,300));       //sitting after lunch
            Wait(Uniform(30, 100));     //leaving
         
            Leave(restaurant);
            timeIntoSystem(Time - tvstup);
        } else { 
            counterLeavs++;
        }
         

    }
};



class PriperdFoodWatchDog : public Process {
    void Behavior() {
        if (priperdFood.Length() > 0){
            for (int i = 0; i < WAITERS_SIZE; i++) {
                if (!waiters[i].Busy()) {
                    Food * meal = (Food *)priperdFood.GetFirst();
                    meal->indexActWaiter = i;
                    waiters[i].QueueIn(meal, 4);
                    break;
                }
            }
        }

        if (priperdFood.Length() > 0){
            for (int i = 0; i < WAITERS_SIZE; i++) {
                if (!waiters[i].Busy()) {
                    Food * meal = (Food *)priperdFood.GetFirst();
                    meal->indexActWaiter = i;
                    waiters[i].QueueIn(meal, 4);
                    break;
                }
            }
        }
        Activate(Time + 1); 
       
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







int main(){
    int numbOfExperiment = 0; 
    scanf("%d",&numbOfExperiment);
    
   
    //RandomSeed(Time(NULL));


    switch(numbOfExperiment){
        /* ---------------------------------------------- */
        /*              TEST 0                            */
        /* node  */
        /* -----------------------------------------------*/
        case 0:
            printf("0. Test starting ... ");
            WAITERS_SIZE = 2;

            restaurant.SetCapacity(30);
            soupKitchem.SetCapacity(3);
            mainCourseKitchen.SetCapacity(5);

            KITCH_MAIN_COUR_SIZE = 3;
            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            (new PriperdFoodWatchDog)->Activate();
            Run();
            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 1                      */
        /* node  */
        /* -----------------------------------------------*/
        case 1:
            printf("1. Experiment starting ... ");
            WAITERS_SIZE = 3;
          
            restaurant.SetCapacity(30);
            soupKitchem.SetCapacity(3);
            mainCourseKitchen.SetCapacity(5);

            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            Run();
            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 2                      */
        /* node  */
        /* -----------------------------------------------*/
        case 2:
            WAITERS_SIZE = 2;
           
            restaurant.SetCapacity(40);
            soupKitchem.SetCapacity(3);
            mainCourseKitchen.SetCapacity(5);

            waiters = new Facility[WAITERS_SIZE];
            Init(0, 14400);
            (new Generator)->Activate();
            Run();
            break;

        /* ---------------------------------------------- */
        /*              EXPERIMENT 3                      */
        /* node  */
        /* -----------------------------------------------*/
        case 3:
            



            break;
        /* ---------------------------------------------- */
        /*              EXPERIMENT 4                      */
        /* node  */
        /* -----------------------------------------------*/
        case 4:
            printf("4. Experiment starting ... ");



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

    waiters[0].Output();
    waiters[1].Output();
    timeIntoSystem.Output();
    supeDelay.Output();
    mainCourseDelay.Output();
    wait1.Output();
    wait2.Output();
    //kitchen.Output();
    printf("Counter of prepared soups: %d\n", counterSoup);
    printf("Counter of prepared main courses: %d\n", counterMainCourses);
    printf("Counter of prepared drinks: %d\n", drinks);
    printf("Leave people:  %d\n", counterLeavs);
    printf("Zisk:  %d\n", (counterSoup * 15) + (counterMainCourses * 70) + drinks * 30);
   


}





