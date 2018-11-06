#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_CHAR 1000 /* Max characters in a line.     */
#define MAX_QUER  100 /* Max character in a query.     */
#define MAX_LINE   10 /* Max number of retained lines. */

#define DEBUG   0 /* Includes score matrix for each line. */
#define VOLUBLE 1 /* Includes Stage 1,2 and 4 output.     */

/*********************************************************************/

/* Structure of a query. */
typedef struct
{	int  tlen;                      /* Query length.            */
	char text[MAX_QUER];            /* Query query text.        */
	int  rlen;                      /* Reduced query length.    */
	char redu[MAX_QUER];            /* Reduced query.           */
	int  scor[MAX_QUER * MAX_QUER]; /* Score tally for Stage 4. */
} quer_t;

/* Structure of a line. */
typedef struct
{	int    numb;           /* Line number.        */
	int    tlen;           /* Line length.        */
	double scor;           /* Line score.         */
	char   text[MAX_CHAR]; /* Line text.          */
	int    rlen;           /* Reduced line length */
	char   redu[MAX_CHAR]; /* Reduced line text.  */
} line_t;

/*********************************************************************/

/* Function prototypes. */
void buildQuery(quer_t *quer, char *find);
void buildLine(line_t *line, char *text);
void reduceLine(line_t *line);
void reduceQuery(quer_t *quer);
void lineScore2(line_t *line, quer_t *quer);
void lineScore4(line_t *line, quer_t *quer);
void scoreRank(line_t *line, line_t *rank);
void insertionSort(line_t *rank);
void swapLine(line_t *a, line_t* b);
void printStage0(quer_t *quer);
void printStage1(line_t *line);
void printStage2(line_t *line);
void printStage3(line_t *rank);
void printStage4(line_t *line);
void printMatrix(quer_t *quer);

/*********************************************************************/

/* Processes input text stage by stage. */
int main(int argc, char **argv)
{	line_t line;               /* Stores current line.           */
	line_t rank[MAX_LINE];     /* Stores 10 highest line scores. */
	quer_t quer;               /* Stores the query.              */
	memset(rank, '\0', sizeof(rank));
	buildQuery(&quer, argv[1]);                     /* Stage 0.  */
	for (line.numb = 1; ; line.numb ++)
	{	buildLine(&line, argv[argc - 1]);           /* Stage 1.  */
		if (line.tlen == 0)                         /* Empty.    */
		{	continue;
		}
		if (line.numb == 0)                         /* Finished  */
		{	printStage3(rank);
			break;
		}
		lineScore2(&line, &quer);                   /* Stage 2.  */
		lineScore4(&line, &quer);                   /* Stage 4.  */
		scoreRank(&line, rank);                     /* Stage 3.  */
	}
	return 0;
}

/*********************************************************************/

/* Stage 0. Generates attributes of the query into quer structure. */
void buildQuery(quer_t *quer, char *find)
{	quer->tlen = strlen(find);
	/* Guard for empty query. */
	if (quer->tlen == 0)
	{	exit(EXIT_FAILURE);
	}
	strcpy(quer->text, find);
	reduceQuery(quer);
	printStage0(quer);
}

/*********************************************************************/

/* Stage 1. Generates attributes of each line into line structure. */
void buildLine(line_t *line, char *text)
{	int i = 0;
	while ((line->text[i] = (char)getchar()) != EOF)
	{	/* Line stops at newline character. */
		if (line->text[i] == '\n')
		{	line->text[i + 1] = '\0';
			line->tlen = i;
			/* Empty line. */
			if (line->tlen == 0)
			{	return;
			}
			reduceLine(line);
			#if (VOLUBLE)
			printStage1(line);
			#endif
			return;
		}
		i ++;
	}
	/* Terminates loop in main.    */
	line->numb = 0;
}

/*********************************************************************/

/* Reduces query to alphanumeric, lowercase characters. */
void reduceQuery(quer_t *quer)
{	int i, j = 0;
	for (i = 0; i < quer->tlen; i ++)
	{	if (isalnum(quer->text[i]))
		{	quer->redu[j] = tolower(quer->text[i]);
			j ++;
		}
	}
	quer->redu[j] = '\0';
	quer->rlen = j;
}

/*********************************************************************/

/* Reduces line to lowercase characters excluding spaces */
void reduceLine(line_t *line)
{	int i, j = 0;
	for (i = 0; i < line->tlen; i ++)
	{	line->redu[j] = tolower(line->text[i]);
		j ++;
	}
	line->redu[j] = '\0';
	line->rlen = j;
}

/*********************************************************************/

/* Stage 2. Computes score of the longest sequence of the query *
 * matched in the current line                                  */
void lineScore2(line_t *line, quer_t *quer)
{	int    i, j, k;
	double coun = 0.0;
	line->scor = 0.0;
	/* Iterates through query. */
	for (i = 0; i < quer->tlen; i ++)
	{	j = i;
		/* Iterates through line. */
		for (k = 0; k < line->tlen; k ++)
		{	/* Found a match! */
			if (line->text[k] == quer->text[j])
			{	coun ++;
				/* Longest match so far. */
				if (coun > line->scor)
				{	line->scor = coun;
				}
				/* Checks next value to retain j if needed. */
				if (line->text[k + 1] != quer->text[j + 1])
				{	coun = 0,0;
				}
				else
				{	j ++;
				}
			}
			/* Rechecks start of query */
			else
			{	coun = 0.0;
				j = i;
			}
		}
	}
	#if (VOLUBLE)
	printStage2(line);
	#endif
}

/*********************************************************************/

/* Stage 4. Computes a score matrix and calculates scores from *
 * matching tri-grams (and larger) based on the formula given  */
void lineScore4(line_t *line, quer_t *quer)
{	int i, j, k;
	line->scor = 0.0;
	memset(quer->scor, '\0', sizeof(quer->scor));
	/* Iterates through query. */
	for (i = 0; i < quer->rlen; i ++)
	{	j = i;
		/* Iterates through line. */
		for (k = 0; k < line->rlen; k ++)
		{	/* Character matches start of query. */
			if (j > i && quer->redu[i] == line->redu[k])
			{	quer->scor[(i * quer->rlen) + i] ++;
			}
			/* Continues matching */
			if (quer->redu[j] == line->redu[k])
			{	quer->scor[(i * quer->rlen) + j] ++;
				j ++;
			}
			/* Recheck query again. :( */
			else
			{	j = i;
			}
		}
		/* Time to add the scores! */
		for (j = i; j < quer->rlen; j ++)
		{	if (j > i + 1 && i < quer->rlen - 2
                && quer->scor[(i * quer->rlen) + j] > 0)
			{	line->scor += (j - i - 1) * (j - i - 1)
                * (log(2 + quer->scor[(i * quer->rlen) + j]) / log(2));
			}
		}
	}
	/* Final division. */
	line->scor /= (log(30 + line->tlen) / log(2));
	#if (DEBUG)
	printMatrix(quer);
	#endif
	#if (VOLUBLE)
	printStage4(line);
	#endif
}

/*********************************************************************/

/* Retains the 10 highest-scoring lines in decreasing order. */
void scoreRank(line_t *line, line_t *rank)
{	/* Lowest score replaced if a higher is found. */
	if (line->scor > rank[MAX_LINE - 1].scor)
	{	rank[MAX_LINE - 1] = *line;
		insertionSort(rank);
	}
}

/*********************************************************************/

/* Reverse sort the array of highest-ranked lines. */
void insertionSort(line_t *rank)
{	int i, j;
	for (i = 1; i < MAX_LINE; i ++)
	{	j = i;
		while (j > 0 && rank[j - 1].scor < rank[j].scor)
		{	swapLine(&rank[j - 1], &rank[j]);
			j --;
		}
	}
}

/*********************************************************************/

/* Used by insertionSort to swap line_t elements */
void swapLine(line_t *a, line_t* b)
{	line_t tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

/*********************************************************************/

/* Prints Stage 0. */
void printStage0(quer_t *quer)
{	printf("S0: query = %s \n", quer->text);
}

/*********************************************************************/

/* Prints Stage 1. */
void printStage1(line_t *line)
{	printf("---\n%sS1: line %5d, bytes = %2d\n",
           line->text, line->numb, line->tlen);
}

/*********************************************************************/

/* Prints Stage 2. */
void printStage2(line_t *line)
{	printf("S2: line %5d, score = %6.3f\n", line->numb, line->scor);
}

/*********************************************************************/

/* Prints Stage 3. */
void printStage3(line_t *rank)
{	int i;
	printf("------------------------------------------------\n");
	/* Ensures printed scores are positive. */
	for (i = 0; i < MAX_LINE && rank[i].scor; i ++)
	{	printf("S3: line %5d, score = %6.3f\n%s---\n",
               rank[i].numb, rank[i].scor, rank[i].text);
	}
}

/*********************************************************************/

/* Prints Stage 4. */
void printStage4(line_t *line)
{	printf("S4: line %5d, score = %6.3f\n", line->numb, line->scor);
}

/*********************************************************************/

/* Prints the score matrix for each line during debugging. */
void printMatrix(quer_t *quer)
{	int i, j;
	printf("DB:   ");
	for (i = 0; i < quer->rlen; i ++)
	{	printf("%c  ", quer->redu[i]);
	}
	for (i = 0; i < quer->rlen; i ++)
	{	printf("\nDB: ");
		for (j = 0; j < quer->rlen; j ++)
		{	printf("%3d", quer->scor[(i * quer->rlen) + j]);
		}
	}
	printf("\n");
}
