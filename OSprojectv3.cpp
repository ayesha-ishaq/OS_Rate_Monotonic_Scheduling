#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <iomanip>

using namespace std;
//	GLOBAL VARIABLES
//
int n_per=1;	//	Number of hyperperiods
int ticks = 0;	//	Number of computer ticks
int curr = 0;	//	current thread to run
int numTasks = 0;
int lcm;
int tt=0;

sem_t* sem;	//	semaphore: allow multiple threads to access the shared memory
sem_t* semCon;	//	semaphore than controls the multiple threads semaphore
struct taskProc* t;	//	each task is of type of taskProc

typedef struct taskProc{
    char name[2];	//	Name of Task
    int exec_t;	//	The exec_t of task
    int per;	//	The period length of task
    int used_time;	// 	The current location, or how much time was applied towards task
    int counter;	//	Counter to keep track of time into the task's period
} taskProc;

//FUNCIONS
void writeTime()
{
  int i;
  cout<<"    ";
	for (i = 0; i < lcm; i++)
	{
		//	For numbers of two digits, remove one space to make it easier to read
		cout << i+1 << setw(5)<<right;
	}
	//write(sched, "\n", strlen("\n"));
  cout << endl;
}

//	----------------------------------------------------------------------------
//	geeksforgeeks.com
//	Function to return gcd of a and b
int gcd(int a, int b)
{
    if (a == 0)
        return b;
    return gcd(b % a, a);
}

//	----------------------------------------------------------------------------
//	geeksforgeeks.com
//	Function to find lcd of array of numbers
int findLCM()
{
    // Initialize result
    int ans = t[0].per;

    // ans contains LCM of arr[0], ..arr[i]
    // after i'th iteration,
    int i;
    for (i = 1; i < numTasks; i++)
        ans = (((t[i].per * ans)) / (gcd(t[i].per, ans)));

    return ans;
}

//	----------------------------------------------------------------------------
//	geeksforgeeks.com
//	Recursive Bubble Sorting Algorithm
void bubbleSort(int n)
{
    // Base case
    if (n == 1)
        return;

    // One pass of bubble sort. After
    // this pass, the largest element
    // is moved (or bubbled) to end.
    int i;
    for (i = 0; i < n - 1; i++)
		{
      if (t[i].per > t[i+1].per)
			{
				struct taskProc temp = t[i];
				t[i] = t[i+1];
				t[i+1] = temp;
			}
		}

    // Largest element is fixed,
    // recur for remaining array
    bubbleSort(n-1);
}
//	============================================================================
//	Input: None, uses global variables.
//	Output:	Checks if schedulable based off of the formula:
//              C1/P1 + C2/P2 + . . . + Cn/Pn < n.(2^(1/n)-1)
//
void checkSche()
{
  float sum = 0.0;  // sum of each task
  float finalSum = 0.0; //  sum of all the tasks
  int i;
  for(i = 0; i < numTasks; i++)
  {
    sum = ((float)t[i].exec_t / (float)t[i].per);
    finalSum += sum;
  }

  double n=numTasks;
  double check=numTasks*((pow(2,1/n))-1);//rate monotonic equation
  if(finalSum > check)
  {
    cout<<"Schedule is not possible.  "<<finalSum<<" > "<<check<<endl;
    delete t;	//	Free the thread memory
  	delete sem;
  	delete semCon;	//	Free the thread semaphores memory
    exit(EXIT_FAILURE);
  }
}

//	============================================================================
//	Input: None, uses global variables.
//	Output:	Simulate RM Scheduling via threads. Each thread will be called to
//					write out to terminal at the time that it was called.
//
void scheduler()
{
	//----------------------------------------------------------------------------
	//	Calculate LCM of tasks' periods
	lcm = findLCM();
	//	Sort the tasks and prioritize by least period time
	bubbleSort(numTasks);

	//	Write time row to file
	//writeTime();
	cout<<endl<<"Ticks  Threads"<<endl;
	//----------------------------------------------------------------------------
  bool cont = 0;	//	flag to break from loop
  int n, i, j, k, l, m;
  for(n = 0; n < n_per; n++)
	{
		//	Main loop - Loop until ticks have reached the lcm
		while(ticks != lcm) //each hyperloop contains ticks equal to least common multiple of all tasks' periods
		{
			//	Loop through the sorted tasks
			for(i = 0; i < numTasks; i++) //in order of priority set by period after bubble sort
			{
				//	loop through number of remaining ticks until thread's exec_t is reached
				for(j = t[i].used_time; j < t[i].exec_t; j++) //cont flag will break out of this loop if any tasks' period has been reached
				{
					sem_post(&sem[i]);	//	unlock determined thread
					sem_wait(semCon);	//	wait until thread gives access back to main loop

					//	Iterate clock ticks and current thread's used processing time
					ticks++;tt++;
					t[i].used_time++;

					//	Iterate all threads' counters and check if any have reach their
					//	period, if so then break out.
					for(k = 0; k < numTasks; k++)
					{
						t[k].counter++; 
						if(t[k].counter == t[k].per)
						{
							//	Reset thread's values if it has reached end of its period
							t[k].used_time = 0;//allow that thread to take complete exec time in next period time
							t[k].counter = 0;//reset counter to keep track of how many time units of period passed 
							i = -1; //to allow tasks to start from highest priority again
							cont = 1;
						}
					}

					//	If previous loop determined that thread needs to break, then break
					//	next loop level
					if(cont )
					{
						cont = 0;
						break;
					}
				}
			}

			//	Continue iterating counters even if exec_t has been reached for all tasks
			for(l = 0; l < numTasks; l++)
			{
				t[l].counter++;
				if(t[l].counter == t[l].per)
				{
					//	Reset thread's values if it has reached end of its period
					t[l].used_time = 0;
					t[l].counter = 0;
				}
			}

			//	Write blank if no thread needs to be called
      cout << tt<< setw(10)<<internal<<"--"<<endl ;
			//	Iterate clock ticks
			ticks++;tt++;
		}

		//	Reset all thread's values when starting next hyperperiod
		for(m = 0; m < numTasks; m++)
		{
			t[m].used_time = 0;
			t[m].counter = 0;
			ticks = 0;
		}
    
	}
  cout<< endl;
	printf("Scheduler has finished successfully.\n");
	curr = -1;	//	Tell threads to exit
}
//	THREAD: task
//
//	Input: Argument passed from thread creation. It is the ID number for the thread
//	Output:	Write name of thread to output file when called by scheduler.
//
void *task(void *vargp)
{
	//	Set thread id to passed argument (id from creating the thread)
	int id = *((int *)vargp);

	//	While scheduling is running
	while(curr != -1)
	{
		//	Thread will wait until scheduler releases it
		if(sem_wait(&sem[id])!= 0)
		{
			cout<<"Semaphore error in thread " << id << "\n";
			exit(EXIT_FAILURE);
		}

		//	If scheduler has finished, then don't write to file and exit loop
		if(curr != -1)
    {
     cout <<tt<< setw(10)<<internal<< t[id].name<<endl;
  		//	Release the semaphore controler
  		sem_post(semCon);
    }
	}

	pthread_exit(0);
}
//	===========================================================================
//	MAIN
//
int main()
{
  //	The thread identifiers
	pthread_t* pt;	//	task thread

  //  --------------------------------------------------------------------------
  //  Input File Management
	ifstream infile; 
   infile.open("info.txt"); 
 
   cout << "Reading from the file" << endl; 
	string fl;

  char* name_ = new char[sizeof(char)*2];	//	Character buffers for file search
	int x = 0;	//	iterator for file
	int exec_t_, period;	//	temp values for data from file

	//	Count number of tasks in file
	while (getline(infile, fl))
   {
		numTasks++;
	}
	cout<<"No. of tasks: "<<numTasks<<endl;
	//	Allocate memory of task object for the number of tasks found
	t = new taskProc[sizeof(taskProc)*numTasks];

	//	Reset file pointer to beginning
	infile.clear();
	infile.seekg(0,infile.beg);

	while(x<numTasks)
	{
		infile >> name_; 
		for(int q=0; name_[q]!=0; q++)
		t[x].name[q] = name_[q];
		infile >> exec_t_;
		t[x].exec_t = exec_t_;
		infile >> period;
		t[x].per = period;
		x++;
	}
	//	Read data from each line of file
	for(int j=0;j<numTasks;j++)
	{
		cout<<"\n"<<t[j].name<<" "<<t[j].exec_t<<" "<<t[j].per;
	}
	cout<<endl;
    //  Close the input file
	infile.close();


	//	INITIALIZATION ===========================================================
	//	Allocate memory for semaphores
	sem = new sem_t[sizeof(sem_t)*numTasks];	//	Size of n tasks, one for each thread
	semCon = new sem_t[sizeof(sem_t)];	//	Separate control semaphore to control the sem
	//	Allocate memory for threads
	pt = new pthread_t[sizeof(pthread_t)*numTasks];	// Size of n tasks

	//	--------------------------------------------------------------------------
	//	Set all thread semaphores to 0, locked
  int i;
	for (i = 0; i < numTasks; i++)
	{
		sem_init(&sem[i], 0, 0);
	}
	//	Set control semaphore to 0, locked
	sem_init(semCon, 0 , 0);


	//	Create n threads
		// grabs current i, if code not here it'd be off by 1
	for (i = 0; i < numTasks; i++)
	{
		int* id = new int[sizeof(int)];	// grabs current i, if code not here it'd be off by 1
		*id = i;
		pthread_create(&pt[i], NULL, task, id);
	}


	//	--------------------------------------------------------------------------
	//	Start the scheduler program
    checkSche();
	scheduler();

	//	--------------------------------------------------------------------------
	//	Allow remaining threads to exit
	for (i = 0; i < numTasks; i++)
			sem_post(&sem[i]);

	//	Join n threads
	for (i = 0; i < numTasks; i++)
		pthread_join(pt[i], NULL);

	//	CLEANUP ==================================================================
	delete t;	//	Free the thread memory
	delete sem;	//	Free the thread semaphores memory
 	delete semCon;

	return 0;
}


//	============================================================================




