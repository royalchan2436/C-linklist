#define _GNU_SOURCE
#include "lists.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void delete_labels(Poll *head, int num);
Poll *find_pre_poll(char *name, Poll *head);
void delete_part(Poll *head);
void free_myself(Poll *head);
char *summary_helper(Poll *head, int position);
void malloc_labels(Poll *head, int num_slots, char **labels, char *mode);
/* Create a poll with this name and num_slots. 
 * Insert it into the list of polls whose head is pointed to by *head_ptr_add
 * Return 0 if successful 1 if a poll by this name already exists in this list.
 */
int insert_new_poll(char *name, int num_slots, Poll **head_ptr_add) {
    
    if(!(find_poll(name, *head_ptr_add))) //Check Poll name not exist
    {
       Poll *new_node = malloc(sizeof(Poll));
       if(!new_node)
       {
	        perror("malloc fail\n");
       }
       new_node->next = NULL;
       malloc_labels(new_node, num_slots, NULL, "set");
       new_node->num_slots = num_slots;
       new_node->participants = NULL;
       strcpy(new_node->name, name);
       Poll *temp = *head_ptr_add;
       if(!temp)    //Case 1: No any Poll 
       { 
	       *head_ptr_add = new_node;
         return 0;
       }
       while(temp->next)  //Case 2: Append in the end
       {
	        temp = temp->next;
       }
       temp->next = new_node;
       return 0;
    }
    return 1;
}


/* Return a pointer to the poll with this name in
 * this list starting with head. Return NULL if no such poll exists.
 */
Poll *find_poll(char *name, Poll *head) {
    
    Poll *temp = head;
    if(!temp) //If NULL pointer
    {
      return NULL;
    }
    while(temp) // Find the Poll
    {
      if(strcmp(temp->name, name) == 0)
      {
	       return temp;
      }
      temp = temp->next;
    }
    return NULL;
}

/* 
 *  Print the names of the current polls one per line.
 */
void print_polls(Poll *head) {
     
     
     Poll *current = head;
     if(current) //If Poll is not NULL
     {
       while(current->next)
       {
      	   printf("%s\n", current->name);
      	   current = current->next;
       }
       printf("%s\n", current->name);
     }
     else //If Poll is NULL
     {
        printf("No Polls\n");
     }

}



/* Reset the labels for the poll by this poll_name to the strings in the
 * array slot_labels.
 * Return 1 if poll does not exist by this name. 
 * Return 2 if poll by this name does not match number of labels provided
 */
int configure_poll(char *poll_name, char **slot_labels, int num_labels, 
                   Poll *head_ptr) {

    Poll *current = find_poll(poll_name ,head_ptr);
    if(!current) //Case 1: NULL pointer
    {
       return 1;
    }
    else if(current->num_slots != num_labels) //Number of labes is not match
    {
       return 2;
    }
    else //Set the labels
    {
       delete_labels(current, num_labels);
       malloc_labels(current, num_labels, slot_labels, "reset");
    }
    return 0;
}


/* Delete the poll by the name poll_name from the list at *head_ptr_add.
 * Update the head of the list as appropriate and free all dynamically 
 * allocated memory no longer used.
 * Return 0 if successful and 1 if poll by this name is not found in list. 
 */
int delete_poll(char *poll_name, Poll **head_ptr_add) {
    
    Poll *current = find_poll(poll_name, *head_ptr_add), *check = *head_ptr_add, *argg = *head_ptr_add;
    if(!current) //Case 1: NULL Pointer Or Not Found
    {  
       return 1;
    }
    if(strcmp(check->name, poll_name) == 0)  //Case 2: Only One Node
    {
       if(!check->next)
       {
          *head_ptr_add = NULL;
          free_myself(check);
          return 0;
       }
    }
    if(current->next) //Case 3: Middle Node and Head Node
    {  
       check = find_pre_poll(poll_name, argg); 
       if(!check)  
       {
          *head_ptr_add = current->next;
          free_myself(current);
          return 0;
       }
       check->next = current->next;
       free_myself(current);
       return 0;
    }
    if(!current->next) //Case 4: Tail Node
    {
       check = find_pre_poll(poll_name, argg);
       check->next = NULL;
       free_myself(current);
       return 0;
    }
    return 1;
}


/* Add a participant with this part_name to the participant list for the poll
   with this poll_name in the list at head_pt. Duplicate participant names
   are not allowed. Set the availability of this participant to avail.
   Return: 0 on success 
           1 for poll does not exist with this name
           2 for participant by this name already in this poll
           3 for availibility string is wrong length for this poll. 
             Particpant not added
*/
int add_participant(char *part_name, char *poll_name, Poll *head_ptr, char* avail) {
    
    Poll *current = find_poll(poll_name, head_ptr);
    if(!current) // NULL pointer
    {
       return 1;
    }
    Participant *p_ptr = find_part(part_name, current);
    if(p_ptr) // Participant exist
    {
       return 2;
    }

    p_ptr = current->participants;  //Create New Participant

    Participant *temp = malloc(sizeof(Participant));
    if(!temp)
    {
      perror("malloc fail\n");
      exit(1);
    }
    if(!(temp->availability = malloc(sizeof(char) * strlen(avail))))
    {
      perror("malloc fail\n");
      exit(1);
    }
    strcpy(temp->availability, avail);
    strcpy(temp->name, part_name);
    temp->next = NULL;
    temp->comment = NULL;

    while(p_ptr) //Add Participant into Poll
    {   
       if(!p_ptr->next)
       {   
           p_ptr->next = temp;
           return 0;
       }
       p_ptr = p_ptr->next;
    }
    current->participants = temp;
    return 0;
}


/* Add a comment from the participant with this part_name to the poll with
 * this poll_name. Replace existing comment if one exists. 
 * Return values:
 *    0 on success
 *    1 no poll by this name
 *    2 no participant by this name for this poll
 */
int add_comment(char *part_name, char *poll_name, char *comment, Poll *head_ptr) {

    Poll *poll = find_poll(poll_name, head_ptr);
    if(!poll) // NULL pointer
    {
       return 1;
    }
    Participant *current = find_part(part_name, poll);
    if(current)
    { 
      if(current->comment) // If comment exist
      {
         if(realloc(current->comment ,sizeof(char) * strlen(comment)) == NULL)
         {
            perror("malloc fail\n");
            exit(1);
         }
         strcpy(current->comment, comment);
         return 0;
      }
      else // Add the first comment
      {
         if((current->comment = malloc(sizeof(char) * strlen(comment))) == NULL)
         {
            perror("malloc fail\n");
            exit(1);
         }
         strcpy(current->comment, comment);
         return 0;
      }
    }
    return 2;
}

/* Add availabilty for the participant with this part_name to the poll with
 * this poll_name. Return values:
 *    0 success
 *    1 no poll by this name
 *    2 no participant by this name for this poll
 *    3 avail string is incorrect size for this poll 
 */
int update_availability(char *part_name, char *poll_name, char *avail, 
          Poll *head_ptr) {

    Poll *p_ptr = find_poll(poll_name, head_ptr);
    if(p_ptr) // Not NULL Pointer
    {  
       Participant *p_p_ptr = find_part(part_name, p_ptr);
       if(p_p_ptr) //Participant Exist
       {  
          free(p_p_ptr->availability);
          if((p_p_ptr->availability = malloc(sizeof(char) * strlen(avail))) == NULL)
          {
              perror("malloc fail\n");
              exit(1);
          }
          strcpy(p_p_ptr->availability, avail);
          return 0;
       }
       return 2; //Participant Not Exist
    }
    return 1; //Poll Not Exist
}


/*  Return pointer to participant with this name from this poll or
 *  NULL if no such participant exists.
 */
Participant *find_part(char *name, Poll *poll) {

    Participant *p_ptr = poll->participants;
    while(p_ptr) // Find the participant
    {  
       if(strcmp(p_ptr->name, name) == 0)
       {
          return p_ptr;
       }
       p_ptr = p_ptr->next;
    }
    return NULL; // Not Found
}
    

/* For the poll by the name poll_name from the list at head_ptr,
 * prints the name, number of slots and each label and each participant.
 * For each participant, prints name and availability.
 * Prints the summary of votes returned by the poll_summary function.
 * See assignment handout for formatting example.
 * Return 0 if successful and 1 if poll by this name is not found in list. 
 */
int print_poll_info(char *poll_name, Poll *head_ptr) {
    
    Poll *current = find_poll(poll_name, head_ptr);
    char *temp_hold;
    if(current) //Not NULL Pointer
    {
       Participant *p_ptr = current->participants;
       while(p_ptr) //Not NULL Pointer
       {
          if(p_ptr->comment) //If Comment Exist
          {
             printf("Participant: %s \t %s\n", p_ptr->name, p_ptr->availability);
             printf("\t Comment: %s\n", p_ptr->comment);
          }
          else //If Comment Not Exist
          {
             printf("Participant: %s \t %s\n", p_ptr->name, p_ptr->availability);
          }
          p_ptr = p_ptr->next;
       }
       temp_hold = poll_summary(current); //Call Helper Function To Print Summary
       printf("\n \n %s\n", temp_hold);
       free(temp_hold); // Deallocate Memory From Helper Function
       return 0;
    }
    return 1;
}


/* Builds and returns a string for this poll that summarizes the answers.
 * Summary information includes the total number of Y,N and M votes
 * for each time slot. See an example in the assignment handout.
 */
char *poll_summary(Poll *poll) {
    char *result = NULL;
    Poll *current = poll;
    int i, size;
    char *temp;
    char *summary = "Availability Summary\n";
    size = strlen(summary) + 1;
    if(current) //Not NULL Pointer
    {  
       //Allocate Memory For A Char ** To Hold Summarizes
       char **hold = malloc(sizeof(char *) * current->num_slots); 
       if(!hold)
       {
          perror("malloc fail\n");
          exit(1);
       }
       for(i = 0; i < current->num_slots; i++)
       {
          temp = summary_helper(current, i); //Get A Sentence From Helper Function And Store in Char **
          hold[i] = malloc(sizeof(char) * strlen(temp)+1);
          if(!hold[i])
          {
             perror("malloc fail\n");
             exit(1);
          }
          strcpy(hold[i], temp);
          size += (strlen(temp) + 1); //Cumulate Each Sentence Length
          free(temp); //Deallocate Memory The Sentence
       }
       result = malloc(sizeof(char) * size);  //Allocate Memory For Result
       if(!result)
       {
           perror("malloc fail\n");
           exit(1);
       }
       strcat(result, summary);
       for(i = 0; i < current->num_slots; i++)
       {
          strcat(result, hold[i]); //Store All Sentence In Result
       }
       for(i = 0; i < current->num_slots; i++) //Deallocate All Used Memory
       {
          free(hold[i]);
       }
       free(hold);
    }
    return result;
}

/* Summary Helper Function */

char *summary_helper(Poll *head, int position)
{
   Poll *current = head;
   int yes = 0, no = 0, maybe = 0;
   char *temp;
   Participant *p_ptr = current->participants;
   while(p_ptr) //Not NULL Pointer And Count Votes
   {
      if(strncmp((p_ptr->availability)+position, "Y", 1) == 0) 
      {
         yes++;
      }
      if(strncmp((p_ptr->availability)+position, "N", 1) == 0)
      {
         no++;
      }
      if(strncmp((p_ptr->availability)+position, "M", 1) == 0)
      {
         maybe++;
      }
      p_ptr = p_ptr->next;
   }
   //Format One Sentence And Return This Sentence
   if(asprintf(&temp, "%s \t Y:%d N:%d M:%d\n", (current->slot_labels)[position], yes, no, maybe) < 0)
   {
      perror("malloc fail\n");
      exit(1);
   }
   return temp;
}

/* Free the input Poll's memory */

void free_myself(Poll *head)
{
    Poll *current = head;
    delete_part(current);
    delete_labels(current, current->num_slots);
    free(current);
}

/* Delete input Poll's labels */

void delete_labels(Poll *head, int num)
{
    Poll *current = head;
    if(current->slot_labels) //If Lables Exist
    {
        int i;
        for(i = 0; i < num; i++)
        {
           free((current->slot_labels)[i]);
        }
        free(current->slot_labels);
    }
}

/* Delete input Poll's participants */

void delete_part(Poll *head)
{
     Poll *temp = head;
     if(temp) // Not NULL Pointer
     {
        Participant *current = temp->participants, *delete;
        while(current) // Not NULL Pointer
        {
           delete = current;
           current = current->next;
           free(delete->availability);
           free(delete->comment);
           free(delete);
        }
        head->participants = NULL;
     }
     
}

/* Find the pre Poll of the input Poll*/

Poll *find_pre_poll(char *name, Poll *head)
{
   Poll *current = head, *pre = head;
   if(!current) // NULL pointer
   {
      return NULL;
   }
   if(!current->next)   //Case 1: Only 1 Node
   {
      if(strcmp(name, current->name) == 0)
      {
         return NULL;
      }
   }
   if(strcmp(name, current->name) == 0) //Case 2: Head Node
   {
      return NULL;
   }
   while(current) // Case 3: Middle Node or Tail Node
   {
      if(strcmp(current->name, name) == 0)
      {  
         return pre;
      }
      pre = current;
      current = current->next;
   }
   return NULL;
}

/* Malloc memory for slot labels */

void malloc_labels(Poll *head, int num_slots, char **slot_labels, char *mode)
{
   Poll *current = head;
   int i;
   char *name = "unconfigured";
   current->slot_labels = malloc(sizeof(char *) * num_slots);
   if(!current->slot_labels)
   {
      perror("malloc fail\n");
      exit(1);
   }
   for(i = 0; i < num_slots; i++)
   {
      if(strcmp(mode, "reset") == 0)
      {
         (current->slot_labels)[i] = malloc(sizeof(char) * strlen(slot_labels[i])+1);
         if(!(current->slot_labels)[i])
         {
           perror("malloc fail\n");
           exit(1);
         }
         strcpy((current->slot_labels)[i], slot_labels[i]);
      }
      else
      {
          (current->slot_labels)[i] = malloc(sizeof(char) * strlen(name) + 1);
          if(!(current->slot_labels)[i])
          {
            perror("malloc fail\n");
            exit(1);
          }
          strcpy((current->slot_labels)[i], name);
       }
   }
}
