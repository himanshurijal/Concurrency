// Copyright (c) 2020 Trevor Bakker, Copyright (c) 2020 Himanshu Rijal
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
  
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3          /* Number of seats in the professor's office */
#define professor_LIMIT 10   /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  	 /* Maximum number of students in the simulation */
#define consecutive_LIMIT 5  /* Maximum number of consecutive students from a class allowed to enter the office */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3
#define CLASSE 4

/* TODO */
/* Add your synchronization variables here */

sem_t office; /* Mutex to guard the critical section */
sem_t enter; /* Mutex to guard the critical section while students check whether or not they should enter the office */


/* Basic information about simulation.  They are printed/checked at the end 
 * and in assert statements during execution.
 *
 * You are responsible for maintaining the integrity of these variables in the 
 * code that you develop. 
 */

static int students_in_office;   /* Total number of students currently in the office */
static int classa_inoffice;      /* Total number of students from class A currently in the office */
static int classb_inoffice;      /* Total number of students from class B in the office */
static int students_since_break; /* Total number of students since the last break */

static int classa_students;      /* Total number of students from class A that have arrived */
static int classb_students;      /* Total number of students from class B that have arrived */
static int classa_consecutive;   /* Consecutive number of students from class A that have entered the office */
static int classb_consecutive;   /* Consecutive number of students from class B that have entered the office */

typedef struct
{
  int arrival_time;  /* time between the arrival of this student and the previous student */
  int question_time; /* time the student needs to spend with the professor */
  int student_id;
  int class;
} student_info;

/* Called at beginning of simulation.  
 * TODO: Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename)
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0; 

  /* Initialize your synchronization variables (and 
   * other variables you might use) here
   */

  classa_students = 0;
  classb_students = 0;
  classa_consecutive = 0;
  classb_consecutive = 0;

  sem_init(&office, 0, MAX_SEATS);  /* Initialize office to MAX_SEATS alloted + 1 for the professor */
                                    /* Second param = 0 - semaphore is local */
  sem_init(&enter, 0, 1);  			/* Initialize enter to one student (Only one student can check for eligibility to enter the office at a time) */

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL)
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) &&
           i < MAX_STUDENTS )
  {
    i++;
  }

 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break.
 */
static void take_break()
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.
 */
void *professorthread(void *junk)
{
  printf("The professor arrived and is starting his office hours\n");

  /* Loop waiting for students to arrive. */
  while (1)
  {

  	/* TODO */
    /* Add code here to handle the student's request. */

    if(students_since_break == 10 && students_in_office == 0)
    {
      take_break();
    }
  }
  pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
 * You have to implement this.
 */
void classa_enter()
{

  /* TODO */
  /* Request permission to enter the office. You might also want to add  */
  /* synchronization for the simulation variables below                  */
  /* YOUR CODE HERE.                                                     */

  classa_students += 1;

  while(1)
  {
    /* Up semaphore */
    sem_wait(&enter);

    if(classb_inoffice != 0 || students_since_break == professor_LIMIT || (classa_consecutive == consecutive_LIMIT && classb_students > 0))
    {
      /* Wait if students from class B are in office or number of students served since break is equal     */
      /* to professsor_LIMIT or 5 (consecutive_LIMIT) consecutive students from class A have entered       */
      /* office while students from B are waiting.														   */ 

      /* Down sempahore */
      sem_post(&enter);
    }
    else
    {
      break;
    }
  }

  sem_wait(&office);

  students_in_office += 1;
  students_since_break += 1;
  classa_inoffice += 1;

  if(classa_consecutive == consecutive_LIMIT && classb_inoffice == 0)
  {
    /* Until a student from class B arrives and enters the office the consecutive        */
    /* number of students from class A that have entered the office will not             */
    /* increase or be reset once it reaches consecutive_LIMIT even if more students      */
    /* from class A enter the office.  													 */
  }
  else
  {
    classa_consecutive += 1;
  }

  sem_post(&enter);

}

/* Code executed by a class B student to enter the office.
 * You have to implement this.
 */
void classb_enter()
{

  /* TODO */
  /* Request permission to enter the office.  You might also want to add  */
  /* synchronization for the simulations variables below                  */
  /* YOUR CODE HERE.                                                      */

  classb_students += 1;

  /* Students from class B will follow similar logic to enter the office as students from class A */
  while(1)
  {

    sem_wait(&enter);

    if(classa_inoffice != 0 || students_since_break == professor_LIMIT || (classb_consecutive == consecutive_LIMIT && classa_students > 0))
    {
      sem_post(&enter);
    }
    else
    {
      break;
    }
  }

  sem_wait(&office);

  students_in_office += 1;
  students_since_break += 1;
  classb_inoffice += 1;

  if(classb_consecutive == 5 && classa_inoffice == 0)
  {

  }
  else
  {
    classb_consecutive += 1; 
  }

  sem_post(&enter);

}

/* Code executed by a student to simulate the time he spends in the office asking questions
 */
static void ask_questions(int t)
{
  sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * You need to implement this.
 */
static void classa_leave()
{
  /*
   *  TODO
   *  YOUR CODE HERE.
   */
    
  students_in_office -= 1;
  classa_inoffice -= 1;
  classa_students -= 1;
  classb_consecutive = 0; /* Once a student from class A has entered and left the office */
  						  /* reset the consecutive count of students from class B        */

  sem_post(&office);

}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.
 */
static void classb_leave()
{
  /*
   * TODO
   * YOUR CODE HERE.
   */
    
  students_in_office -= 1;
  classb_inoffice -= 1;
  classb_students -= 1;
  classa_consecutive = 0; /* Once a student from class B has entered and left the office */
  						  /* reset the consecutive count of students from class A        */

  sem_post(&office);
}

/* Main code for class A student threads.
 */
void* classa_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter();

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );
  
  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classa_leave();

  printf("Student %d from class A leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter();

  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();

  printf("Student %d from class B leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 * GUID: 355F4066-DA3E-4F74-9656-EF8097FBC985
 */
int main(int nargs, char **args)
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2)
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0)
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);

  result = pthread_create(&professor_tid, NULL, professorthread, NULL);

  if (result)
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++)
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);
                
    student_type = random() % 2;

    if (s_info[i].class == CLASSA)
    {
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result)
    {
      printf("officehour: thread_fork failed for student %d: %s\n",
            i, strerror(result));
      exit(1);
    }
  }

  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++)
  {
    pthread_join(student_tid[i], &status);
  }

  /* tell the professor to finish. */
  pthread_cancel(professor_tid);
    
    /* destroy semaphore */
    sem_destroy(&office);
    sem_destroy(&enter);
    
  printf("Office hour simulation done.\n");

  return 0;
}
