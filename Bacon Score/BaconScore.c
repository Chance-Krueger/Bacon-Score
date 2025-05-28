/*
* File: bacon.c
* Author: Chance Krueger
* 
* Purpose: 
*   This program parses a dataset containing movie-actor relationships, 
*   constructing a graph where actors serve as nodes and movies create 
*   connections between them. 
*   
*   Using Breadth-First Search (BFS), it calculates the "Bacon Number," 
*   representing the shortest path (degrees of separation) between a 
*   given actor and Kevin Bacon.
* 
*   The program builds an undirected graph, marking connections through 
*   shared movie appearances. If an actor has a valid path to Kevin Bacon, 
*   it returns the number of connections required to reach him; otherwise, 
*   it outputs "No Bacon!" to indicate the absence of a link.
* 
*   Inspired by the "Oracle of Bacon," which popularized the concept, 
*   this program allows users to determine their degrees of separation 
*   from Kevin Bacon based on custom movie-actor data.
*/




#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>



void printActorsWithMovies();



/*
 * actorNode -- Represents an actor in the graph of actors and movies.
 *
 * Fields:
 *   actorName - Pointer to a dynamically allocated string containing the actor's name.
 *   movies    - Pointer to a linked list (of type movieList) representing the movies
 *               in which the actor has appeared.
 *   next      - Pointer to the next actorNode in the overall linked list of actors.
 *   visited   - A flag used during graph traversal (e.g., BFS) to indicate whether
 *               this actor has been visited.
 *   level     - An integer used during graph traversal that indicates the distance
 *               (or “level”) from a start node.
 */
struct actorNode {

	char *actorName;
	struct movieList *movies; // char *movieName;
	struct actorNode *next;
	int visited;
	int level;
};



/*
 * movieNode -- Represents a movie in the graph connecting actors.
 *
 * Fields:
 *   movieName - Pointer to a dynamically allocated string containing the movie's name.
 *   actors    - Pointer to a linked list (of type actorsInMovie) representing the actors
 *               that appear in the movie.
 *   next      - Pointer to the next movieNode in the overall linked list of movies.
 */
struct movieNode {

	char *movieName;
	struct actorsInMovie *actors;
	struct movieNode *next;
};



/*
 * movieList -- Represents a node in an actor's list of movies.
 *
 * Fields:
 *   movie - Pointer to a movieNode that represents a movie in which the actor has appeared.
 *   next  - Pointer to the next node in the linked list of movies for the actor.
 */
struct movieList {
	struct movieNode *movie;
   	struct movieList *next;
};



/*
 * actorsInMovie -- Represents a node in a movie's list of actors.
 *
 * Fields:
 *   to   - Pointer to an actorNode representing an actor who appears in the movie.
 *   next - Pointer to the next node in the linked list of actors for the movie.
 */
struct actorsInMovie {
	struct actorNode *to;
	struct actorsInMovie *next;
};


/*
* queue -- node for a simple linked-list queue used in BFS.
* pNode: pointer to the phoneNode stored in this queue entry.
* next: pointer to the next entry in the queue.
*/
struct queue {
        struct actorNode *actor;
        struct queue *next;
};



/*
* dequeue(head) -- removes and returns the front phoneNode from the queue.
* head: pointer to the queue head pointer.
* Returns: the dequeued phoneNode*, or NULL if the queue was empty.
* Assumptions: *head is a valid pointer; queue entries have been allocated.
* Side effects: frees the removed queue node.
*/
struct actorNode* dequeue(struct queue **head) {
    if (*head == NULL) {
        return NULL;  // Return NULL if the queue is empty.
    }
    struct actorNode *node = (*head)->actor;
    struct queue *temp = *head;
    *head = (*head)->next;  // Move the head pointer to the next element
    free(temp);
    return node;
}



/*
* enqueue(head, pNode) -- creates a new queue entry holding pNode and appends it.
* head: pointer to the queue head pointer.
* pNode: pointer to the phoneNode to enqueue.
* Returns: void.
* Assumptions: head points to a valid queue pointer; memory allocation succeeds.
* Side effects: allocates a new queue node and links it at the tail.
*/
void enqueue(struct queue **head, struct actorNode *actor) {
    struct queue *node = malloc(sizeof(struct queue));
    node->actor = actor;
    node->next = NULL;

    if (*head == NULL) {
        *head = node;  // If the queue is empty, the new node is the head.
        return;
    }

    struct queue *cur = *head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = node;  // Add the new node to the end of the queue
}


// Main linked lists
struct actorNode *headActors = NULL;
struct movieNode *headMovies = NULL;



/*
* findMovie(line) -- extracts the movie title from a formatted input string.
* line: pointer to a string containing the full movie entry (e.g., "Movie: Title").
* Returns: a newly allocated string containing the movie title, with the prefix removed.
* Assumptions: line contains a ':' separating "Movie" from the title.
* Side effects: dynamically allocates memory for the returned string; caller must free it.
*/
char* findMovie(char *line) {

	char *movie = strdup(line);
	char *og = movie;

	while (*movie != ':') {
		movie++;
	}
	// GET RID OF SPACE BEFORE ':'
	movie += 2;
	char* duped = strdup(movie);
	free(og);
	return duped;
}


/*
* containsMovie(line) -- determines whether a given string contains a movie entry.
* line: pointer to a string that may represent a movie or actor entry.
* Returns: 1 if the string contains a movie identifier (":"), otherwise 0.
* Assumptions: line is a valid, null-terminated string.
* Side effects: none.
*/
int containsMovie(char *line) {

	for (int index = 0; line[index] != 0; index++) {
		if (line[index] == ':') {
			return 1;
		}
	}
	return 0;
}




/*
* addMovieToActorsMovies(actor, movie) -- links a movie to an actor's list of appearances.
* actor: pointer to the actorNode representing the actor.
* movie: pointer to the movieNode representing the movie.
* Returns: void.
* Assumptions: actor and movie pointers are valid and properly allocated.
* Side effects: dynamically allocates memory for a new movieList node and appends it
*               to the actor's movie list.
*/
void addMovieToActorsMovies(struct actorNode *actor, struct movieNode *movie) {
	
	struct movieList *curMovie = actor->movies;

	struct movieList *addMovie = malloc(sizeof(struct movieList));
	
	if (addMovie == NULL) {
		fprintf(stderr, "Not enough Mem.\n");
		exit(1);
	}

	addMovie->movie = movie;
	addMovie->next = NULL;
	
	if (curMovie == NULL) {
		actor->movies = addMovie;
		return;
	}

	while (curMovie->next != NULL) {
		curMovie = curMovie->next;
	}

	curMovie->next = addMovie;
}



/*
* addMovieToLL(node) -- adds a movieNode to the global linked list of movies.
* node: pointer to the movieNode to be inserted.
* Returns: void.
* Assumptions: node is a valid, dynamically allocated movieNode.
* Side effects: updates headMovies if the list is empty, appends node at the tail.
*/
void addMovieToLL(struct movieNode *node) {

	if (headMovies == NULL) {
                headMovies = node;
                node->next = NULL;
                return;
        }

        struct movieNode *cur = headMovies;

        while (cur->next != NULL) {
                cur = cur->next;
        }
        cur->next = node;
        node->next = NULL;


}



/*
* addActorNode(node) -- adds an actorNode to the global linked list of actors.
* node: pointer to the actorNode to be inserted.
* Returns: void.
* Assumptions: node is a valid, dynamically allocated actorNode.
* Side effects: updates headActors if the list is empty, appends node at the tail.
*/
void addActorNode(struct actorNode *node) {
	if (headActors == NULL) {
		headActors = node;
		node->next = NULL;
		return;
	}

	struct actorNode *cur = headActors;

	while (cur->next != NULL) {
		cur = cur->next;
	}
	cur->next = node;
	node->next = NULL;
}



/*
* addActorToMovie(movie, actor) -- adds an actor to a movie's linked list of actors.
* movie: pointer to the movieNode representing the movie.
* actor: pointer to the actorNode representing the actor.
* Returns: void.
* Assumptions: movie and actor pointers are valid and properly allocated.
* Side effects: dynamically allocates a new actorsInMovie node and appends it to
*               the movie's list of actors, ensuring no duplicate actors are added.
*/
void addActorToMovie(struct movieNode *movie, struct actorNode *actor) {

	struct actorsInMovie *curActor = movie->actors;

	struct actorsInMovie *newActor = malloc(sizeof(struct actorsInMovie));

	if (newActor == NULL) {
		fprintf(stderr, "Not Enough Space.\n");
		exit(1);
	}

	newActor->to = actor;
	newActor->next = NULL;

	if (curActor == NULL) {
		movie->actors = newActor;
		return;
	}

	while (curActor->next != NULL) {
		// COPY, DONT ADD, SHOULD NEVER BE TRUE
		if (strcmp(curActor->to->actorName, actor->actorName) == 0) {
			return;
		}
		curActor = curActor->next;
	}
	curActor->next = newActor;
}



/*
* findActor(actor) -- searches for an actor in the global linked list of actors.
* actor: pointer to a string containing the actor's name.
* Returns: pointer to the actorNode if found, otherwise NULL.
* Assumptions: headActors is a valid pointer to the linked list of actors.
* Side effects: none.
*/
struct actorNode* findActor(char *actor) {
	struct actorNode *cur = headActors;

        while (cur != NULL) {
                if (strcmp(cur->actorName, actor) == 0) {
                        return cur;
                }
                cur = cur->next;
        }
        return NULL;
}



/*
* isInLL(actor) -- checks whether an actor is present in the global linked list.
* actor: pointer to a string containing the actor's name.
* Returns: 1 if the actor exists in the linked list, otherwise 0.
* Assumptions: headActors points to a valid linked list of actorNode structures.
* Side effects: none.
*/
int isInLL(char *actor) {
	struct actorNode *cur = headActors;

        while (cur != NULL) {
	
                if (strcmp(cur->actorName, actor) == 0) {
			return 1;
		}
		
		cur = cur->next;

	}
	return 0;
}

	

/*
* parseFile(file) -- reads and processes a file containing movie and actor information.
* file: pointer to an open FILE stream containing movie-actor data.
* Returns: void.
* Assumptions: file is a valid pointer to an open file.
* Side effects: dynamically allocates memory for movie and actor nodes, modifies global
*               linked lists (headMovies and headActors), and frees allocated strings.
*/
void parseFile(FILE *file) {

	char *movieTitle = NULL;

	struct movieNode *movie = NULL;
	

	char *line = NULL;
	size_t size = 0;

	while ((getline(&line, &size, file)) > 0) {
		
		// NEW LINE, GO TO NEXT ITERATION/LINE
		if (isspace(line[0])) {
			continue;
		}

		// Get rid of newline char
		if (line[strlen(line) - 1] == '\n') {
    			line[strlen(line) - 1] = '\0';
		}

		// If it contains a ':'
		if (containsMovie(line)) {
			
			// ADD MOVIE TO MOVIELL AND ADD MOVIE TO EVERY ACTORS MOVIE LIST
                        if (movie != NULL) {
				addMovieToLL(movie);
                                free(movieTitle);
                        	movieTitle = NULL;
                        }

			movieTitle = (findMovie(line));

			movie = malloc(sizeof(struct movieNode));
			
			if (movie == NULL) {
                                fprintf(stderr, "Not Enough Memory.\n");
                        	exit(1);
                        }
			
			movie->movieName = strdup(movieTitle);
			movie->next = NULL;
			movie->actors = NULL;

		} else {
			// ADD NODE
			
			// NODE ALREADY EXISTS
			if (isInLL(line)) {
				struct actorNode *actor = findActor(line);
				addMovieToActorsMovies(actor, movie);
				addActorToMovie(movie, actor);
			} else {
				// CREATE NODE
				struct actorNode *actor = malloc(sizeof(struct actorNode));
				
				if (actor == NULL) {
					fprintf(stderr, "Not Enough Memory.\n");
					exit(1);
				}
				
				actor->actorName = strdup(line);
				actor->movies = NULL;
				actor->next = NULL;
				actor->movies = NULL;
				actor->visited = 0;
				actor->level = 0;
				addActorNode(actor);
				addMovieToActorsMovies(actor, movie);
				addActorToMovie(movie, actor);
			}
		}
	}
	if (movieTitle != NULL) {
		addMovieToLL(movie);
                free(movieTitle);
                movieTitle = NULL;
        }
	free(line);
}



/*
* freeActorsInMovie(head) -- frees the linked list of actorsInMovie nodes.
* head: pointer to the first node in the actorsInMovie linked list.
* Returns: void.
* Assumptions: head is either NULL or points to a valid linked list.
* Side effects: deallocates all nodes in the linked list but does not free actorNode pointers.
*/
void freeActorsInMovie(struct actorsInMovie *head) {
    	while (head != NULL) {
        	struct actorsInMovie *temp = head;
        	head = head->next;
        	free(temp);  // 'to' is not owned — don't free it!
    	}
}



/*
* freeActorMovieList(head) -- frees the linked list of movieList nodes belonging to an actor.
* head: pointer to the first movieList node.
* Returns: void.
* Assumptions: head is either NULL or points to a valid linked list.
* Side effects: deallocates all movieList nodes but does not free movieNode pointers,
*               as they are managed separately in freeMovieList.
*/
void freeActorMovieList(struct movieList *head) {
    	while (head != NULL) {
        	struct movieList *temp = head;
        	head = head->next;
        	// DO NOT free(temp->movie) — not owned, handled in freeMovieList
        	free(temp);
    	}
}



/*
* freeActorList(head) -- frees all actorNode structures in the linked list.
* head: pointer to the first actorNode in the list.
* Returns: void.
* Assumptions: head is either NULL or points to a valid linked list.
* Side effects: deallocates all actor nodes, freeing actorName strings and
*               associated movie lists. Does not free movieNode pointers directly.
*/
void freeActorList(struct actorNode *head) {
    	while (head != NULL) {
        	struct actorNode *temp = head;
        	head = head->next;
        	free(temp->actorName);
        	freeActorMovieList(temp->movies);  // Correct type now
        	free(temp);
    	}
}



/*
* freeMovieList(head) -- frees all movieNode structures and their actor lists.
* head: pointer to the first movieNode in the list.
* Returns: void.
* Assumptions: head is either NULL or points to a valid linked list.
* Side effects: deallocates all movie nodes, freeing movieName strings and
*               associated actorsInMovie lists. Does not free actorNode pointers.
*/
void freeMovieList(struct movieNode *head) {
    	while (head != NULL) {
        	struct movieNode *temp = head;
        	head = head->next;
        	free(temp->movieName);
        	freeActorsInMovie(temp->actors);  // Don't free actorNode pointers
        	free(temp);
    	}
}



/*
* printActorsWithMovies() -- prints a list of all movies and their associated actors.
* Returns: void.
* Assumptions: headMovies points to a valid linked list of movieNode structures.
* Side effects: outputs movie names and their actor lists to standard output.
* Note: This function is primarily used for debugging and verifying movie-actor relationships.
*/
void printActorsWithMovies() {

    struct movieNode *curMovie = headMovies;

    while (curMovie != NULL) {
	
	    printf("MOVIE: %s\n", curMovie->movieName);

	    struct actorsInMovie *curActor = curMovie->actors;

	    while (curActor != NULL) {
		    printf("	ACTOR: %s\n", curActor->to->actorName);
	    	    curActor = curActor->next;
	    }
	    curMovie = curMovie->next;
    }
}



/*
* freeQueue(head) -- frees all queue nodes in a linked list queue structure.
* head: pointer to the first queue node.
* Returns: void.
* Assumptions: head is either NULL or points to a valid queue linked list.
* Side effects: deallocates all queue nodes but does not free actorNode pointers.
*/
void freeQueue(struct queue *head) {
    while (head != NULL) {
        struct queue *temp = head;
        head = head->next;
        free(temp);
    }
}



/*
* BFS(start, target) -- performs Breadth-First Search to find the shortest path 
*                        (in terms of degrees of separation) between two actors.
* start: pointer to the actorNode representing the starting actor.
* target: pointer to the actorNode representing the target actor.
* Returns: the shortest path length (number of connections) or -1 if no path exists.
* Assumptions: headActors is a valid pointer to the global linked list of actors.
* Side effects: modifies actorNode fields (visited and level) and dynamically allocates 
*               memory for the queue, which is freed before returning.
*/
int BFS(struct actorNode *start, struct actorNode *target) {
    
	if (start == target) {
		return 0;
	}

    	// Clear all visited and levels
    	struct actorNode *cur = headActors;
    	while (cur != NULL) {
        	cur->visited = 0;
        	cur->level = -1;
        	cur = cur->next;
    	}

    	struct queue *q = NULL;
    	start->visited = 1;
    	start->level = 0;
    	enqueue(&q, start);

    	while (q != NULL) {
        	struct actorNode *a = dequeue(&q);

        	// Loop over all movies this actor is in
        	struct movieList *ml = a->movies;
        	while (ml != NULL) {
            		struct movieNode *movie = ml->movie;

            		// Loop over all actors in this movie
            		struct actorsInMovie *co = movie->actors;
            		while (co != NULL) {
                		struct actorNode *c = co->to;

                		if (!c->visited) {
                    			c->visited = 1;
                    			c->level = a->level + 1;

                    			if (c == target) {
                        			freeQueue(q);
						return c->level;
                    			}
                    			enqueue(&q, c);
                		}

                		co = co->next;
            		}
            		ml = ml->next;
        	}
    	}
	freeQueue(q);
    	return -1; // Not found
}




/*
* main(argc, argv) -- reads movie-actor data from a file and determines the 
*                     degrees of separation from Kevin Bacon using BFS.
* argc: integer representing the number of command-line arguments.
* argv: array of strings representing the command-line arguments.
* Returns: 0 on successful execution, 1 if errors occur (file issues, invalid actors).
* Assumptions: argv[1] contains a valid filename; input format follows expected structure.
* Side effects: allocates memory for actors and movies, modifies global linked lists,
*               dynamically reads from stdin, and frees allocated resources before exiting.
*/
int main(int argc, char* argv[]) {

	FILE *file; 
	int minusOption = 0;

	int errSeen = 0;

	int fileNotInit = 0;
	int mult = 0;
	for (int index = 1; index < argc; index++) {
	
		if (strcmp("-l", argv[index]) == 0) {
			minusOption = 1;
			mult++;
			if (mult > 1) {
				fprintf(stderr, "Too many optional Arguments.\n");
				return 1;
			}
		} else {
			if (!fileNotInit) {
				file = fopen(argv[index], "r");
				fileNotInit++;
			} else {
				fprintf(stderr, "Too many Files were given.\n");
				return 1;
			}
		}
	}

	minusOption++;
	
	if (file == NULL) {
		fprintf(stderr, "Could not Open the File.\n");
		return 1;
	}

	parseFile(file);
	
	char *actorName = NULL;
	size_t len = 0;
	struct actorNode *actor;

	while ((getline(&actorName, &len, stdin)) > 0) {

		if (actorName[strlen(actorName) - 1] == '\n') {
			actorName[strlen(actorName) - 1] = '\0';
        	}

		if ((actor = findActor(actorName)) == NULL) {
			errSeen = 1;
			fprintf(stderr, "Actor Could Not be Found.\n");
			continue;
		}

		struct actorNode *bacon = findActor("Kevin Bacon");

		// No Bacon in Graph, Dont have to check it.
		if (bacon == NULL) {
			printf("Score: No Bacon!\n");
			continue;
		}

		int bfs = BFS(bacon, actor);
	
		if (bfs == -1) {
			printf("Score: No Bacon!\n");
		} else {
			printf("Score: %d\n", bfs);
		}
	}
	free(actorName);
	freeActorList(headActors);
	freeMovieList(headMovies);
	fclose(file);
	return errSeen;
}
