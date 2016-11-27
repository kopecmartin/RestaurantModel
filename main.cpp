#include <simlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACITY 30
#define KITCHEN_SLOTS 6
#define WAITERS 2

Histogram dobaVSystemu("Celkova doba v systemu", 0, 900, 16);
Store restaurant("Store represented restaurant whit maximal capacity", CAPACITY);
Store kitchen("Store represented parallel prepar food in kitchen", KITCHEN_SLOTS);
Facility waiters[WAITERS].SetName("Waithers of restaurant");
Queue waitForSoup;
Queue waitForMainCourse;
Queue orders;

int counterLeavs = 0;
int counterMainCourses = 0;
int counterSoup = 0;
int drinks = 0;


class Food : public Process {
public: 
	bool manualyActivate;
	bool isSoup;

	Food(bool soup) : Process() {
		isSoup = soup;
		manualyActivate = false;
	}

	void Behavior() {
		
		Enter(kitchen);
		// printf("Enter\n");
		if (isSoup) {
			counterSoup++;
			Wait(Uniform(100,200));
		} else {
			counterMainCourses++;
			Wait(Uniform(400,500));
		}
		Leave(kitchen);
		int who = -1;
		if (waiters[1].Busy()) {
			for (int i = 0; i < WAITERS; i++) {
				if (!waiters[i].Busy()) {
					who = i;
					Seize(waiters[i]);
					break;
				}
			}
		}
		if (who == -1) {
			who = 1;
			Seize(waiters[who], 1);
		}
		if (!manualyActivate){
			if (waiters[1].Q1->Length() > 0){
				// printf("Get 2 meal\n");
				Food * meal = (Food *)waiters[1].Q1->GetFirst();
				meal->manualyActivate = true;
				Wait(Uniform(20,25));
				meal->Activate();
			} else {
				Wait(Uniform(15,20));
			}

			

			Release(waiters[who]);
	 	}
	 	// Impatient food
	 	while (true){
		 	if ( isSoup ){
		 		if (waitForSoup.Length() > 0){
					waitForSoup.GetFirst()->Activate();
					break;
				} else {
					Wait(10);
					continue;
				}
			} else {
				if (waitForMainCourse.Length() > 0){
					waitForMainCourse.GetFirst()->Activate();
					break;
				} else {
					Wait(10);
					continue;
				}
			}
		}
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

	void Behavior() {
		//TODO may be interupted
		Seize(waiters[whoWaiter], 1);
		printf("New\n");
		int drinksOrderd = 0;
		if (!mnaualyFree){
			int orderCounter = 1;
			if (whoWaiter == 0 && waiters[0].Q1->Length() > 0){
				printf("I have more orders\n");
				for(int i = 0; i < (int) waiters[0].Q1->Length(); i++){
					orderCounter++;
					Order * ord = (Order *)waiters[0].Q1->GetLast();
					ord->mnaualyFree = true;
					ord->Activate();
					if (Random() < 0.30)
						drinksOrderd++;


				}
			}
			if (!(foodCombo == 1)){
				(new Food(true))->Activate();
			}

			if (!(foodCombo == 2)){
				(new Food(false))->Activate();
			}
			//pripering drinks
			if (Random() < 0.30)
				drinksOrderd++;
			if (drinksOrderd > 0){
				drinks += drinksOrderd;
				Wait(Uniform(60 * drinksOrderd, 90 * drinksOrderd));
			}

			printf("Waiting mulyiplay %d\n", orderCounter);
			Wait(Uniform(4 * orderCounter, 6 * orderCounter));
			Release(waiters[whoWaiter]);
		} else {
			if (!(foodCombo == 1)){
				(new Food(true))->Activate();
			}
			
			if (!(foodCombo == 2)){
				(new Food(false))->Activate();
			}
		}
		
		// printf("After Seize get order: %d\n", waiters[whoWaiter].Q1->Length());
	}
};





class Guest : public Process {
	void Behavior() {
		if( !restaurant.Full() ){
			int foodCombo = 0;
			float randomNum = Random();
			if (randomNum < 0.85){
				foodCombo = 0;
			} else if(randomNum < 0.98){
				foodCombo = 1;
			} else {
				foodCombo = 2;
			}


			double tvstup = Time;
			Enter(restaurant, 1);

			Wait(Uniform(15,40));	
			Wait(Uniform(80,150));

			int who = -1;
			if (waiters[0].Busy()) {
				for (int i = 1; i < WAITERS; i++) {
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
		
			Wait(Uniform(15,20));	
			
			(new Order(who, foodCombo))->Activate();	//make order
			


			Release(waiters[who]);
			if (!(foodCombo == 1)){
				Into(waitForSoup);
				Passivate();
				Wait(Uniform(200,400));		//eating soup
			}
			if (!(foodCombo == 2)){
				Into(waitForMainCourse);
				Passivate();
				Wait(Uniform(600,1200));		//eating main course
			}
			who = -1;
			if (waiters[0].Busy()) {
				for (int i = 1; i < WAITERS; i++) {
					if (!waiters[i].Busy()) {
						who = i;
						Seize(waiters[i]);
						break;
					}
				}
			}
			
			if (who == -1) {
				who = 0;
				Seize(waiters[who], 3);
			}
			printf("Platenie Who %d    %d\n", who,waiters[0].Q1->Length() );
			Wait(Uniform(20,60));	
			Release(waiters[who]);

			// printf("Thank you by\n");
			Leave(restaurant);
			dobaVSystemu(Time - tvstup);
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

int main(){
	int numbOfExperiment = 0; 
	scanf("%d",&numbOfExperiment);
	//RandomSeed(Time(NULL));


	switch(numbOfExperiment){
		/* ---------------------------------------------- */
		/*   			TEST 0						      */
		/* node  */
		/* -----------------------------------------------*/
		case 0:
			printf("0. Test starting ... ");
			Init(0, 14400);
			//
			(new Generator)->Activate();
			Run();




			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 1				      */
		/* node  */
		/* -----------------------------------------------*/
		case 1:
			printf("1. Experiment starting ... ");




			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 2				      */
		/* node  */
		/* -----------------------------------------------*/
		case 2:
			printf("2. Experiment starting ... ");



			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 3				      */
		/* node  */
		/* -----------------------------------------------*/
		case 3:
			printf("3. Experiment starting ... ");



			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 4				      */
		/* node  */
		/* -----------------------------------------------*/
		case 4:
			printf("4. Experiment starting ... ");



			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 5 				      */
		/* node  */
		/* -----------------------------------------------*/
		case 5:
			printf("5. Experiment starting ... ");	




			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 6				      */
		/* node  */
		/* -----------------------------------------------*/
		case 6:
			printf("6. Experiment starting ... ");




			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 7				      */
		/* node  */
		/* -----------------------------------------------*/
		case 7:
			printf("7. Experiment starting ... ");



			break;
		/* ---------------------------------------------- */
		/*   			EXPERIMENT 8				      */
		/* node  */
		/* -----------------------------------------------*/
		case 8:
			printf("8. Experiment starting ... ");
			break;

	}

	waiters[0].Output();
	waiters[1].Output();
	dobaVSystemu.Output();
	kitchen.Output();
	printf("Counter of prepared soups: %d\n", counterSoup);
	printf("Counter of prepared main courses: %d\n", counterMainCourses);
	printf("Counter of prepared drinks: %d\n", drinks);
	printf("Leave people:  %d\n", counterLeavs);
	printf("Zisk:  %d\n", (counterSoup * 15) + (counterMainCourses * 70) + drinks * 30);

	
	printf("e: %f\n", Exponential(100) );
	printf("e: %f\n", Exponential(0.1) );
	printf("e: %f\n", Exponential(1) );
	printf("e: %f\n", Exponential(2) );

}





