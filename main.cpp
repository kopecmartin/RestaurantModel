#include <simlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACITY 40
#define COOKERS 6

Histogram dobaVSystemu("Celkova doba v systemu", 0, 40, 20);
Store restaurant("Store restaurant whit maximal capacity", CAPACITY);
Store kitchen("Store restaurant whit maximal capacity", COOKERS);
Facility waiter("Waiter who receive orders");
Facility waiter_2("Waiter who distributes food");
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
		printf("Enter\n");
		if (isSoup) {
			Wait(Uniform(100,200));
		} else {
			Wait(Uniform(400,500));
		}

		Leave(kitchen);


		Seize(waiter_2);

		if (!manualyActivate){
			// printf("On way\n");
			Wait(Uniform(5,10));
			if (waiter_2.Q1->Length() > 0){
				// printf("Get 2 meal\n");
				Food * meal = (Food *)waiter_2.Q1->GetFirst();
				meal->manualyActivate = true;
				meal->Activate();
			}
			Release(waiter_2);
	 	}
	 	if ( isSoup ){
	 		printf("Queue waitForSoup : %d\n", waitForSoup.Length());
			waitForSoup.GetFirst()->Activate();
			printf("Queue waitForSoup : %d\n", waitForSoup.Length());
		} else {
			waitForMainCourse.GetFirst()->Activate();
		}
	}
};

class Order : public Process {
	void Behavior() {
		//TODO may be interupted
		// printf("New order\n");
		Seize(waiter,1);
		(new Food(true))->Activate();
		(new Food(false))->Activate();
		Release(waiter);
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

		
			Seize(waiter, 2);
			Wait(Uniform(20,30));	
			(new Order)->Activate();	//make order
			Release(waiter);

			// printf("Wait for soup ...\n");

			Into(waitForSoup);
			Passivate();
			Wait(Uniform(100,150));		//eating soup

			// printf("Wait for main course\n");
			Into(waitForMainCourse);
			Passivate();
			Wait(Uniform(600,1000));		//eating main course

	
			if (!waiter.Busy()) {
				Seize(waiter, 3);
				Wait(Uniform(20,60));	
				Release(waiter);
			} else if(!waiter_2.Busy()){
				Seize(waiter_2);
				Wait(Uniform(20,60));	
				Release(waiter_2);
			} else {
				Seize(waiter, 3);
				Wait(Uniform(20,60));	
				Release(waiter);
			}

			// printf("Thank you by\n");
			Leave(restaurant);
			dobaVSystemu(Time - tvstup);
		} else {
			 
			 i++;
		}
		printf("Leave !!! %d\n", i);

	}
};


class Generator : public Event { 
    void Behavior() {
        (new Guest)->Activate();
        Activate(Time + Uniform(0,150)); 
    }
};


int main(){
	Init(0, 14000);
	(new Generator)->Activate();
	Run();

	dobaVSystemu.Output();
}





