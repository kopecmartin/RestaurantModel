#include <simlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACITY 30
#define KITCHEN_SLOTS 6
#define WAITERS 2

Histogram dobaVSystemu("Celkova doba v systemu", 0, 40, 20);
Store restaurant("Store restaurant whit maximal capacity", CAPACITY);
Store kitchen("Store restaurant whit maximal capacity", KITCHEN_SLOTS);
Facility waiter("Waiter who receive orders");
Facility waiter_2("Waiter who distributes food");
Facility waiters[WAITERS];
Queue waitForSoup;
Queue waitForMainCourse;

int i = 0;



class Food : public Process {

public: 
	bool manualyActivate;
	bool isSoup;

	Food(bool soup) : Process() {
		isSoup = soup;
		manualyActivate = false;
	}

	void Behavior() {
		// printf("New foot\n");
		Enter(kitchen);
		// printf("Enter\n");
		if (isSoup) {
			Wait(Uniform(100,200));
		} else {
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
			printf("Seize obsluha jedla na stol: %d\n", waiters[who].Q1->Length());
			Seize(waiters[who], 1);
		}
	
		if (!manualyActivate){
			if (waiters[1].Q1->Length() > 0){
				printf("Get 2 meal\n");
				Food * meal = (Food *)waiters[1].Q1->GetFirst();
				meal->manualyActivate = true;
				Wait(Uniform(20,25));
				meal->Activate();
			} else {
				Wait(Uniform(15,20));
			}
			Release(waiters[who]);
	 	}
	 	if ( isSoup ){
			waitForSoup.GetFirst()->Activate();
	
		} else {
			waitForMainCourse.GetFirst()->Activate();
		}
	}
};

class Order : public Process {
public:
	int whoWaiter;

	Order(int who) : Process() {
		whoWaiter = who;
	}

	void Behavior() {
		//TODO may be interupted
		// printf("New order\n");
		printf("Before Seize get order: %d\n", waiters[whoWaiter].Q1->Length());
		Seize(waiters[whoWaiter], 2);
		Wait(Uniform(10,20));
		(new Food(true))->Activate();
		(new Food(false))->Activate();
		Release(waiters[whoWaiter]);
		printf("After Seize get order: %d\n", waiters[whoWaiter].Q1->Length());
	}
};


class Guest : public Process {
	void Behavior() {
		if( !restaurant.Full() ){
			
			double tvstup = Time;
			Enter(restaurant, 1);

			// printf("Im here\n");
			Wait(Uniform(15,40));	
			Wait(Uniform(80,150));
			int who = -1;
			if (waiters[0].Busy()) {
				for (int i = 1; i < WAITERS; i++) {
					if (!waiters[i].Busy()) {
						who = i;
						printf("Zakakujem: %d   0\n", waiters[i].Q1->Length());
						Seize(waiters[i]);
						break;
					}
				}
			}
			if (who == -1) {
				who = 0;
				printf("Seize objednavanie: %d   0\n", waiters[0].Q1->Length());
				Seize(waiters[0], 3);
			}
		
			Wait(Uniform(20,30));	
			(new Order(who))->Activate();	//make order
			Release(waiters[who]);

			// printf("Wait for soup ...\n");

			Into(waitForSoup);
			Passivate();
			Wait(Uniform(100,150));		//eating soup

			// printf("Wait for main course\n");
			Into(waitForMainCourse);
			Passivate();
			Wait(Uniform(600,1000));		//eating main course

			
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
			printf("Platenie Who %d    %d\n", who,waiters[0].Q1->Length() );
			if (who == -1) {
				who = 0;
				Seize(waiters[who], 3);
			}
			Wait(Uniform(20,60));	
			Release(waiters[who]);

			

			printf("Thank you by\n");
			Leave(restaurant);
			dobaVSystemu(Time - tvstup);
		} else {
			 
			 i++;
		}
		// printf("Leave !!! %d\n", i);

	}
};


class Generator : public Event { 
    void Behavior() {
        (new Guest)->Activate();
        Activate(Time + Uniform(0,40)); 
    }
};


int main(){
	Init(0, 14000);
	(new Generator)->Activate();
	Run();

	dobaVSystemu.Output();
}





