# RestaurantModel


## About

The project was created as a school project. The aim of the project is to
design and implement a simulation of a restaurant. The data obtained from the
simulation can be usefull to:
- find the slowest operations in the restaurant
- calculate the profit
- determine if more staff/tables can increase the efficiency and the profit

The program uses [simlib](http://www.fit.vutbr.cz/~peringer/SIMLIB/) library
to simulate processes like preparing food, distribution of the food to the
customers, leaving and coming the new customers, ...


## Simulation

The program parameters describe a restaurant where all statistics data were
manually measured, like:
- number of tables, seats, staff
- time needed to prepare the food
- time needed to distribute the food
- average time needed to make an order
- average time needed to create and pay for a bill
- mean time of incomming new customers

The program allows a user to run a several simulations:
1. Simulation with original parameters without a change
2. Simulation with parameters chosen by a user like number of tables, cooks,
waiters, capacity of the restaurant and the number of plates a waiter can carry
at the same time
3. Simulation with parameters chosen by a user but a new ordering system is
used - time needed to make an order is shorter
4. Simulation used to find out how after many days the investment into the new
ordering system will return
5. Simulation allows a user to run it with custom repeats - this will make
the statistics more precise


## Results

We used the simulations to run a few experiments where we customized the
parameters of the restaurant to find out the more efficient and profitable ways
of running the restaurant. Full report of the analysis can be found among
source files - Full\_Analysis\_SK.pdf, however, it's available only in Slovak
language.


## Authors

[Andrej Chudy](https://github.com/chudyandrej)

[Martin Kopec](https://github.com/kopecmartin)
