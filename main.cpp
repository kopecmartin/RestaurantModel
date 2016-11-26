#include <simlib.h>
#include <stdio.h>
#include <string.h>

#define CAPACITY 20
#define COOKERS 2

Histogram dobaVSystemu("Celkova doba v systemu", 0, 40, 20);
Store restaurant("Store restaurant whit maximal capacity", CAPACITY);
Store kitchen("Store restaurant whit maximal capacity", COOKERS);
Facility waiter("Waiter who receive orders");
Facility waiter_2("Waiter who distributes food");
Queue waitForSoup;
Queue waitForMainCourse;



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
		Enter(kitchen, 1);
		if (isSoup) {
			Wait(Uniform(30,40));
		} else {
			Wait(Uniform(100,400));
		}
		Leave(kitchen, 1);
		Seize(waiter_2);
		if (!manualyActivate){
			// printf("On way\n");
			Wait(Uniform(10,20));
			if (waiter_2.Q1->Length() > 0){
				// printf("Get 2 meal\n");
				Food * meal = (Food *)waiter_2.Q1->GetFirst();
				meal->manualyActivate = true;
				meal->Activate();
			}
			Release(waiter_2);
	 	}
	 	if ( isSoup ){
			waitForSoup.GetFirst()->Activate();
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
			Wait(Uniform(10,90));

			// printf("Wait for waiter\n");
			Seize(waiter, 2);
			Wait(Uniform(3,10));
			(new Order)->Activate();	//make order
			Release(waiter);

			// printf("Wait for soup ...\n");
			Into(waitForSoup);
			Passivate();
			Wait(Uniform(60,180));		//eating soup

			// printf("Wait for main course\n");
			Into(waitForMainCourse);
			Passivate();
			Wait(Uniform(250,400));		//eating main course


			if (!waiter.Busy()) {
				printf("Pay for waiter\n");
				Seize(waiter, 3);
				Wait(Uniform(30,60));
				Release(waiter);
			} else if(!waiter_2.Busy()){
				printf("Pay for waiter2\n");
				Seize(waiter_2);
				Wait(Uniform(30,60));
				Release(waiter_2);
			} else {
				Seize(waiter, 3);
				Wait(Uniform(30,60));
				Release(waiter);
			}

			// printf("Thank you by\n");
			Leave(restaurant, 1);
			dobaVSystemu(Time - tvstup);
		} else {
			// printf("Leave !!!\n");
		}
	}
};


class Generator : public Event {
    void Behavior() {
        (new Guest)->Activate();
        Activate(Time + Uniform(0,600));
    }
};


int main(){
	Init(0, 14000);
	(new Generator)->Activate();
	Run();

	dobaVSystemu.Output();
}
