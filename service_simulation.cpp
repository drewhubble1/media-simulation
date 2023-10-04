/****************************************************************************************
    Drew Hubble

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    Instructions:
    1) Create a folder in your current working directory named "output_files" 
    2) Use the following line in your terminal to compile the program:
        g++-12 -o service_simulation.out service_simulation.cpp
    3) Run the program using the following line:
        ./service_simulation.cpp
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    Summary:
        This program is a simulation of a shared streaming service model. The model is a server
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    Sources of Data and Statistics:
    - https://www.newvisiontheatres.com/tv-viewership-statistics
    - https://cultmtl.com/2022/11/market-share-of-streaming-services-netflix-canada-prime-video-disney-plus-crave-apple-tv-paramount-house-of-the-dragon/
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

****************************************************************************************/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <random>
#include <queue>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <fstream>

/*--------------------------GLOBAL CONSTANTS--------------------------*/
const double our_monthly_fee = 20;         // price customer pays for our service
const double mean_service_time = 150;      // mean service time in minutes 
const int num_customers = 10000;           // number of customers in total
const int num_services = 6;                // number of streaming services
const int month_min = 60*24*31;            // one month of simulation in minutes
const int num_months = 2;                  // number of months to run the simulation
bool VIEW_LIVE_TRANSACTIONS = false;       // set to 'true' if you want to view the live transactions in the terminal
/*-------------------------CONST GLOBAL ARRAYS------------------------*/
const std::string service_names[num_services] = {"Netflix", "Disney+", "CraveTv", "Prime", "Paramount+", "AppleTv+"};
const double service_costs[num_services] = {9.99, 11.99, 9.99, 9.99, 9.99, 8.99};
const int service_accounts[num_services] = {300, 200, 200, 200, 50, 50};
/*--------------------------GLOBAL VARIABLES--------------------------*/
int num_interactions = 0;
/*--------------------------------------------------------------------*/


/*--------------------------------RANDOM NUMBER GENERATORS--------------------------------*/
std::default_random_engine generator;
std::exponential_distribution<double> service_dist(1 / mean_service_time);
/*----------------------------------------------------------------------------------------*/


/*-------------------------------CUSTOMER CLASS-------------------------------*/
class Customer {
    public:
        int cust_id;
        int arrival_time;    
        int service_time;
        int depart_time = -1;
        int time_of_queue;
        int delay_time;
        int chosen_service;

        // constructor
        Customer(int cust_id, int arrival_time) {
            this->cust_id = cust_id;
            this->arrival_time = arrival_time;
        }

        void SetServiceTime() {
            //service_time = round(service_dist(generator) + 1);

            // test --> uniform distribution between 0.5 and 3 hours (30 minutes - 180 minutes)
            double u = (double)rand() / (double)RAND_MAX;
            service_time = 30 + (u * 150);
        }

        void SetArrivalTime(int sys_time, int offset = 0) {
            int time;
            int time_of_day = sys_time % 60*24;
            double u = (double)rand() / (double)RAND_MAX;
            if (time_of_day < 60*12) {  // if customer leaves service before 12pm, then
                if (u < 0.1) {         // 10% probability that the customer arrives at a uniformly distibuted time between time_of_day + 12 hours and 1pm
                    time = (sys_time - time_of_day) + rand()%((60*13)-(time_of_day)+1) + (time_of_day);
                }
                else if (u < 0.6) {    // 50% probability that the customer arrives at a uniformly distibuted time between 1pm-9pm
                    time = (sys_time - time_of_day) + rand()%((60*21)-(60*13)+1) + (60*13);
                }
                else {                  // 40% probability that the customer arrives at a uniformly distibuted time between 9pm-1am
                    time = (sys_time - time_of_day) + rand()%((60*25)-(60*21)+1) + (60*21);
                }
            }
            else {                      // if customer leaves service after 12pm, then
                if (u < 0.4) {          // 40% probability that the customer arrives at a uniformly distibuted time between time_of_day and 1am
                    time = (sys_time - time_of_day) + rand()%((60*25)-(time_of_day)+1) + (time_of_day);
                }
                else if (u < 0.5) {     // 10% probability that the customer arrives at a uniformly distibuted time between 1am and 9am
                    time = (sys_time - time_of_day) + 60*24 + rand()%((60*9)-(60)+1) + (60);
                }
                else {                  // 50% probability that the customer arrives at a uniformly distibuted time between 9am and 1pm
                    time = (sys_time - time_of_day) + 60*24 + rand()%((60*13)-(60*9)+1) + (60*9);
                }
            }
            arrival_time = time + offset;
        }

        void ChooseService() {
            double u = (double)rand() / (double)RAND_MAX;
            if (u < 0.3) {          // 30% probability that the customer choses Netflix
                chosen_service = 0;
            }
            else if (u < 0.5) {     // 20% probability that the customer choses Disney+
                chosen_service = 1;
            }
            else if (u < 0.7) {     // 20% probability that the customer choses CraveTv
                chosen_service = 2;
            }
            else if (u < 0.9) {     // 20% probability that the customer choses Prime
                chosen_service = 3;
            }
            else if (u < 0.95) {    // 5% probability that the customer choses Paramount+
                chosen_service = 4;
            }
            else {                  // 5% probability that the customer choses AppleTv+
                chosen_service = 5;
            }
        }

        void ReInitializeCustomer(int sys_time) {
            ChooseService();
            SetArrivalTime(sys_time);
            SetServiceTime();
        }
};
/*---------------------------------------------------------------------------*/


/*------------------------------------STREAMING SERVICE CLASS------------------------------------*/
class StreamingService {
    public:
        int num_accounts;                             // number of accounts for the service
        double cost;                                  // monthly cost for the streaming service
        std::string name;                             // streaming service name
        // queue delay variables
        int num_delays = 0;
        int total_delay = 0;
        int max_delay = 0;
        int time_in_queue = 0;
        int num_active_users = 0;                     // number of active users (initially zero)
        std::queue<Customer*> service_queue;          // queue for users
        Customer* active_users[num_customers];        // array of active users
        
        // function to convert time in int to string with leading zeros
        std::string StringTime(int arg) {
            size_t n = 2;   // date and clock time use 2 digits always (leading zeros)
            return std::string(n - std::min(n, std::to_string(arg).length()), '0') + std::to_string(arg);
        }

        // constructor
        StreamingService(const int num_accounts, const double cost, const std::string name) {
            this->num_accounts = num_accounts;
            this->cost = cost;
            this->name = name;
            for (int i = 0; i < num_customers; i++) {
                active_users[i] = NULL;
            }
        }

        // function to serve customers, or add them to queue if the service is full
        void ServeCustomer(Customer* cust, int sys_time) {
            if (num_active_users < num_accounts) {                  // if the service is available, then
                num_active_users++;                                 // increment the number of active users
                active_users[cust->cust_id] = cust;                 // add user to the active users array
                cust->depart_time = sys_time + cust->service_time;  // calculate the departure time
                num_interactions++;                                 // increase the number of interactions
                if (VIEW_LIVE_TRANSACTIONS == true)
                    std::cout << "\033[1;32mCustomer " << cust->cust_id << " entered service " << name << " at time " << GetDateTime(sys_time) << " and will leave at time " << GetDateTime(cust->depart_time) << "\n";
            }
            else {
                service_queue.push(cust);                           // add customer to queue
                cust->time_of_queue = sys_time;                     // set the time of queue for the customer
                if (VIEW_LIVE_TRANSACTIONS == true)
                    std::cout << "\033[1;33mCustomer " << cust->cust_id << " entered queue for service " << name << " at time " << GetDateTime(sys_time) << "\n";
            }
        }

        // function to release customers that are ready to be released, and serve queued customers when a customer leaves
        void ReleaseCustomer(Customer* cust, int sys_time) {
            active_users[cust->cust_id] = NULL;                         // remove the user from active users array
            num_active_users--;                                         // decrease the active users count
            if (VIEW_LIVE_TRANSACTIONS == true)
                std::cout << "\033[1;31mCustomer " << cust->cust_id << " left service " << name << " at time " << GetDateTime(sys_time) << "\n";
            if (service_queue.size() > 0) {                             // if there is queued customers, then
                Customer *q_cust = service_queue.front();              // get the customer at the front of queue
                service_queue.pop();                                    // remove that customer from the queue
                q_cust->delay_time = sys_time - q_cust->time_of_queue;  // calculate the delay time for the customer
                total_delay += (sys_time - q_cust->time_of_queue);      // increase the total delay time for this service queue
                if (sys_time != q_cust->time_of_queue)                  // if customer spent time in queue, then
                    num_delays++;                                       // increment the number of delays for this service
                if (q_cust->delay_time > max_delay)                     // if customer delay is largest delay, then
                    max_delay = q_cust->delay_time;                     // set the maximum delay to customer delay
                q_cust->ReInitializeCustomer(sys_time);                 // reinitialize the customer (choose service, get service time)
                active_users[q_cust->cust_id] = q_cust;                 // add user to the active users array
                if (service_queue.size() > 0)                           // if the queue is not empty, then
                    time_in_queue++;                                    // increment the time in queue
                if (VIEW_LIVE_TRANSACTIONS == true)
                    std::cout << "\033[1;34mCustomer " << q_cust->cust_id << " left queue for service " << name << " at time " << GetDateTime(sys_time) << "\n";
            }
        }

        // function to get the time as month-day hours:minutes
        std::string GetDateTime(int time) {
            int months = (int)((float)time / (float)month_min);
            int days = (int)((float)(time - months*month_min) / (float)(24*60));
            int hours = (int)((float)(time - months*month_min - days*24*60) / (float)(60));
            int minutes = time - months*month_min - days*24*60 - hours*60;
            size_t n = 2;   // clock time uses 2 digits always (leading zeros)
            return StringTime(months+1) + "/" + StringTime(days+1) + "/2023" + " " + StringTime(hours) + ":" + StringTime(minutes);
        }
        std::string GetTime(int time) {
            int hours = (int)((float)time / (float)60);
            int minutes = time - hours*60;
            return StringTime(hours) + ":" + StringTime(minutes);
        }

        // function for probability of delay
        double ProbDelay() {
            return (double)num_delays / (double)num_interactions;
        }
        // function for average delay time
        double AvgDelay() {
            return (double)total_delay / (double)num_delays;
        }
        // function for queue utilization
        double QueueUtil(int sys_time) {
            return (double)time_in_queue * 100 / (double)sys_time;
        }
};
/*-----------------------------------------------------------------------------------------------*/

double LittlesLaw(int sys_time, int num_delays, double avg_delay) {
    double lambda = (double)num_delays / (double)sys_time;
    return lambda * avg_delay;

}

int main() {
    srand(time(NULL));
    /*---------------------------INITIALIZE STREAMING SERVICES---------------------------*/
    StreamingService* services[num_services];                    // array of the streaming services
    for (int i=0; i < num_services; i++) {
        StreamingService* s_serv;
        s_serv = new StreamingService(service_accounts[i], service_costs[i], service_names[i]);
        services[i] = s_serv;      // fill array with streaming services
    }

    /*-------------------------------INITIALIZE CUSTOMERS--------------------------------*/
    int arrival_of_last_customer = 0;
    Customer* customers[num_customers];                    // array of customers
    for (int i=0; i < num_customers; i++) {
        Customer* cust;
        cust = new Customer(i, 0);
        cust->ChooseService();
        cust->SetArrivalTime(0, i);
        cust->SetServiceTime();
        customers[i] = cust;      // fill array with customers

        // if the customer is the last one to enter service, record their time of arrival
        if (cust->arrival_time > arrival_of_last_customer)
            arrival_of_last_customer = cust->arrival_time;
    }

    /*-------------------------RUN CUSTOMERS THROUGH SIMULATION--------------------------*/
    int sys_time = 0;                    // simulated time

    // open csv file for transactions
    std::ofstream outfile;
    outfile.open ("output_files/transactions.csv");
    // write the data field names
    outfile << "Customer Id, Service Name, Time Of Arrival, Time Of Departure, Minutes In Service, Minutes In Queue\n";

    while (sys_time < num_months*month_min) {
        /* serve new customers */
        for (int i=0; i<num_customers; i++) {
            if (customers[i]->arrival_time == sys_time) {
                services[customers[i]->chosen_service]->ServeCustomer(customers[i], sys_time);
            }
            if (customers[i]->depart_time == sys_time) {
                services[customers[i]->chosen_service]->ReleaseCustomer(customers[i], sys_time);

                // after all customers have entered the system at least once
                if (sys_time >= arrival_of_last_customer) {
                    // write transaction data to csv file 
                    outfile << customers[i]->cust_id << ", ";
                    outfile << services[customers[i]->chosen_service]->name << ", ";
                    outfile << services[0]->GetDateTime(customers[i]->arrival_time) << ", ";
                    outfile << services[0]->GetDateTime(customers[i]->depart_time) << ", ";
                    outfile << customers[i]->service_time << ", ";
                    outfile << customers[i]->delay_time << "\n";
                }

                customers[i]->ReInitializeCustomer(sys_time);
            }
        }
        sys_time++;
    }
    // close the csv file
    outfile.close();


    /* ---------------------- OUTPUT STATS ---------------------- */

    std::cout << "\n\033[0mThe last customer entered the system at: " << services[0]->GetDateTime(arrival_of_last_customer) << "\n";

    /* --------------- COST & REVENUE --------------- */
    
    double total_cost = 0;
    for (int i = 0; i < num_services; i++)
    {
        total_cost += service_costs[i] * service_accounts[i] * num_months;
    }
    double revenue = (double)num_customers * our_monthly_fee * num_months;
    double profit = revenue - total_cost;

    std::cout << std::setprecision(2) << std::fixed;
    std::cout << "\n---  COST & REVENUE RESULTS  ---\n";
    std::cout << "\nCost: $" << total_cost;
    std::cout << "\nRevenue: $" << revenue;
    std::cout << "\nProfit: $" << profit << "\n";

    /* ------------------- QUEING & DELAY ------------------- */

    /* table for service outputs */         // use fstream write to a table in a file
    std::cout << "\n---  SERVICE QUEUEING RESULTS  ---\n";
    std::cout << "\n   Service      Cost       Num_Accounts    Avg_Cust_in_Queue   Queue_Util   Num_Queues   Prob_of_Queue   Avg_Queue(mins)   Max_Queue(mins)\n";
    std::cout << "-------------------------------------------------------------------------------------------------------------------------------------------\n";
    for (int i = 0; i < num_services; i++) {
        std::cout << "  " << std::setw(10) << services[i]->name
            << std::setprecision(2) << std::fixed
            << "    " << std::setw(3) << "$" << services[i]->cost
            << "    " << std::setw(8) << services[i]->num_accounts
            << std::setprecision(4) << std::fixed
            << "    " << std::setw(13) << LittlesLaw(sys_time, services[i]->num_delays, services[i]->AvgDelay())
            << "    " << std::setw(12) << services[i]->QueueUtil(sys_time) << "%"
            << "    " << std::setw(9) << services[i]->num_delays
            << "    " << std::setw(9) << services[i]->ProbDelay()
            << "    " << std::setw(12) << services[i]->AvgDelay()
            << "    " << std::setw(12) << services[i]->max_delay
            << "\n";
    }

    // write the cost data to a csv file
    std::ofstream monetaryfile;
    monetaryfile.open ("output_files/costdata.csv");
    // write the data field names
    monetaryfile << "Total Cost, Revenue, Profit\n";
    monetaryfile << total_cost << ", " << revenue << ", " << profit << "\n";


    // write the service data to a csv file
    std::ofstream servicefile;
    servicefile.open ("output_files/servicedata.csv");
    // write the data field names
    servicefile << "Service Name, Service Cost, Number Of Accounts, Average Queue Contents, Queue Utilization, Instances Of Queue, Probability Of Queue, Average Queue Time, Maximum Queue Time\n";

    for (int i = 0; i < num_services; i++) {
        servicefile << services[i]->name << ", ";
        servicefile << services[i]->cost << ", ";
        servicefile << services[i]->num_accounts << ", ";
        servicefile << LittlesLaw(sys_time, services[i]->num_delays, services[i]->AvgDelay()) << ", ";
        servicefile << services[i]->QueueUtil(sys_time) << ", ";
        servicefile << services[i]->num_delays << ", ";
        servicefile << services[i]->ProbDelay() << ", ";
        servicefile << services[i]->AvgDelay() << ", ";
        servicefile << services[i]->max_delay << "\n";
    }

    return 0;
}