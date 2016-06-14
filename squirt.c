/*does ParseGenome need int command?

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define false 0
#define true 1
#define low 0
#define high 1

#define population 100
#define genomeSize 1000 //should be kept a multiple of ten
#define genomeCommands 13
#define storeDefault 100
#define fireSpeeds 3
#define numberTypes 10
#define memTypes 14
#define genMemTypes 3
#define maxCommandLength 3 //***may need to increase to 5 to allow for branching reactions
#define maxTurns 100
#define maxTax 3
#define actionCommands 3

#define maxGenerations 2
#define shuffleRate 5 //n/geneShuffle genes are mixed between genomes

//print
#define genesPerLine 100
#define genesPerGroup 10

//genome map symbols
const char CMND = 'C';

//genome commands
#define EMPTY 0
#define C0 1//query commands
#define C1 2
#define C2 3
#define C3 4
#define C4 5
#define C5 6//check{material}ask if {low/high} relative to other materials then {action} else (if not true) next command
#define C6 7
#define C7 8
#define C8 9
#define C9 10//
#define WAIT 11//action commands
#define LAUNCH 12
#define RESET 13//resets geneome to index at or before current genome position of command
//JUMP?


typedef struct {

    int id;
    int age;
    int score;
    int *store; //squirt's materials tracked here
    int *genome;
    char *genomeMap; //'C' inserted for beginning of every genome command, subsequent commands are not currently labeled

    //scoring tools
    int launchCntr;

}squirt;


void GenerateGenome(squirt *world);
int CommandGen(int* genome, int g);
void PairSquirts(squirt *abe, squirt *bob);
int ParseGenome(squirt *active, squirt *target, int command, int g);
int CheckSquirt(squirt *s);
int IsLowest(squirt *active, int material);
int IsHighest(squirt *active, int material);
int DrawMaterial(squirt *active, int material, int speed);
void LaunchPacket(squirt *target, int material, int payload);
void MatePair(squirt *active, squirt *passive);
int FindNumCommands(squirt *active);
int* ReturnLargestInt (int *a, int *b);
int ScoreSquirt(squirt *s);
void SortPopulation(squirt *world);
void PrintAverage(squirt *world, int gen);
void PrintGenome(squirt *active);

int main()
{
    srand(time(NULL));


    squirt *world = malloc(sizeof(squirt) * population);

    //generate population
    int g, p, s;
    printf("generating population\n");
    for (p = 0; p < population; p++) {
        world[p].id = p;
        world[p].age = 0;
        world[p].score = 0;

        world[p].launchCntr = 0;

        world[p].store = malloc(sizeof(int) * numberTypes);
        world[p].genome = malloc(sizeof(int)*genomeSize);
        world[p].genomeMap = malloc(sizeof(char)*genomeSize);
        //current = world[i];
        GenerateGenome(&world[p]);

        for (s = 0; s < numberTypes; s++)
        {
            world[p].store[s] = storeDefault;
        }

    }//end for

    printf("population generated\n");

    //simulation loop
    for (g = 0; g < maxGenerations; g++)
    {

        //interaction
        p = 0;
        while (p < population)
        {
            printf("Pairing\n");
            PairSquirts(&world[p], &world[p+1]);
            p+=2;
        }

        printf("Sorting\n");
        //sort population
        SortPopulation(world);

        p = 0;
        while (p < population)
        {
            printf("Mating\n");
            MatePair(&world[p], &world[p+1]);
            p+=2;
        }

    }

    printf("free resources\n");
    //free memory resources
    for (p = 0; p < population; p++) {
        if (world[p].store != NULL)
            free(world[p].store);

        if (world[p].genome != NULL)
            free(world[p].genome);

        if (world[p].genomeMap != NULL)
            free(world[p].genomeMap);
    }

    if (world != NULL)
        free(world);


    return 0;
}


void GenerateGenome(squirt *active)
{
    int g;
    for (g = 0; g < genomeSize; g++)
    {
        active->genome[g] = rand() % genomeCommands + 1; //0 is null
        active->genomeMap[g] = CMND;
        g = CommandGen(active->genome, g);
    }

}

int CommandGen(int* genome, int g)
{
    int command = genome[g];

    switch(command)
    {
        case WAIT:
            break;

        case LAUNCH:
            g++;
            genome[g] = rand() % fireSpeeds + 1;
            g++;
            genome[g] = rand() % numberTypes;
            break;

        case RESET:
            g++;
            genome[g] = rand() % g; //reset index to any number from 0 to current position
            break;

        default:
            g++;
            genome[g] = rand() % high; //if low  or high
            g++;
            genome[g] = rand() % actionCommands + numberTypes; //low returns true if lowest material, high returns true if highest material does not distinguish between equal materials


    }

    return g;
}

/*Function where creature interaction begins
    ga and gb are genome indices which are updated by ParseGenome
*/
void PairSquirts(squirt *abe, squirt *bob) {
    int score;
    int command, ga, gb;
    int taxamnt, taxtyp;
    int time = 0;

    while ((CheckSquirt(abe) == true || CheckSquirt(bob) == true) && (time < maxTurns)) {

        //reread genome from beginning
        if (ga > genomeSize)
        {
            ga = 0;
        }

        if (gb > genomeSize)
        {
            gb = 0;
        }


        command = abe->genome[ga];
        ga = ParseGenome(abe, bob, command, ga);
        command = bob->genome[gb];
        gb = ParseGenome(bob, abe, command, gb);


        //tax creatures
        taxamnt = rand() % maxTax;
        taxtyp = rand() % numberTypes;
        time++;

        abe->score = ScoreSquirt(abe);
        bob->score = ScoreSquirt(bob);
    }

}

/*check squirt for any number stores at 0
if any at 0 squirt is dead, returns false
*/
int CheckSquirt(squirt *s) {

    int i;
    for (i = 0; i < numberTypes; i++)
    {
        if (s->store[i] == 0)
            return false;
    }

    return 1;
}

//reads genome, enacts command. returns
int ParseGenome(squirt *active, squirt *target, int command, int g) {

    int speed;
    int material;
    int payload;

    EXECUTE: switch (command)
    {

    	case EMPTY:
    	    g = g++;
    	    goto EXECUTE; //keeps squirt from losing a turn from empty genome spaces

        case WAIT:
            g++;
            break;

        case LAUNCH:
            g++;
            speed = active->genome[g]; //select speed
            g++;
            material = active->genome[g]; //select material

            payload = DrawMaterial(active, material, speed); //draw speed amount from store

            active->launchCntr += payload; //for scoring

            LaunchPacket(target, material, payload); //fire packet
            break;


        case RESET:
            g++;
            g = active->genome[g]; //set index to new position
            break;

        default:
            material = active->genome[g];
            g++;

            if (active->genome[g] == low)
            {
                if (IsLowest(active, material) == low)
                {
                    //allow it to parse next command (reaction)
                    g++;
                    break;
                }
                else
                {
                    //skip reaction command
                    g += 2;
                    break;
                }
            }

            if (IsHighest(active, material) == high)
            {
                g++;
                break;
            }
            else
            {
                g += 2;
                break;
            }



    }

}

int IsLowest(squirt *active, int material)
{
    int i;
    int matValue = active->store[material];

    for (i = 0; i < numberTypes; i++)
    {
        if (i == material) //skip selected material
            i++;

        if (active->store[i] > matValue)
            i++;
        else
            return false;
    }

    return true;
}

int IsHighest(squirt *active, int material)
{
    int i;
    int matValue = active->store[material];

    for (i = 0; i < numberTypes; i++)
    {
        if (i == material) //skip selected material
            i++;

        if (active->store[i] < matValue)
            i++;
        else
            return false;
    }

    return true;
}

//delete q units from active.store
//return int of materials drawn from active's store
int DrawMaterial(squirt *active, int material, int speed)
{
    //check active store
    if (active->store[material] >= speed)
    {
        //delete from store
        active->store[material] -= speed;
        return speed;
    }
    //active store < q
    else
    {
        int d = speed - active->store[material];
        active->store[material] -= d; //set to 0, draw remaining
        return d;
    }
}

void LaunchPacket(squirt *target, int material, int payload)
{
    target->store[material] += payload;
}

void MatePair(squirt *active, squirt *passive)
{
    //need to exchange commands using genome map
    /*
    1) generate int array of command header locations
    2) whichever squirt has the lowest number of commmands, divide that squirt's commands by 2
    3) exchange that number of genes with each other by swapping command sections
    4) update genome map
    */
    //zero will always be a command

    int Asize, Bsize, exchangeNum;
    int *largest;
    int *genomeStore;

    genomeStore = malloc(sizeof(int) * genomeSize);

    Asize = FindNumCommands(active);
    Bsize = FindNumCommands(passive);

    largest = ReturnLargestInt(&Asize, &Bsize);

    exchangeNum = *largest / 2;

    int i, j, k, g;
    int actIndex = 0, oldActdex = 0, pasIndex = 0, oldPasdex = 0;

    for (i = 0; i < exchangeNum; i++)
    {
        if (active->genomeMap[i] == CMND)
        {
            actIndex = i; //find last active index to copy to
        }

        if (passive->genomeMap[i] == CMND)
        {
            pasIndex = i;
        }



        //copy genome from index to index
        //copy from old index to new index
        //need to keep gaps from appearing in genome

        j = 0;
        k = 0;
        g = 0;



        //copy actGenome to genomeStore
        while (j < actIndex)
        {
            genomeStore[g] = active->genome[j];

            j++;
            g++;
        }

        j = 0;
        while (k < pasIndex)
        {
            active->genome[j] = passive->genome[k];

            j++;
            k++;
        }

        g = 0;
        k = 0;
        while (g < actIndex)
        {
            passive->genome[k] = genomeStore[g];

            g++;
            k++;
        }


    }//end for

    free(genomeStore);
}

int FindNumCommands(squirt *active) //returns number of command headers in a genome map
{
    int i, commands;
    for (i = 0; i < genomeSize; i++)
    {
        if (active->genomeMap[i] == CMND)
            commands++;
    }

    return commands;
}

int* ReturnLargestInt (int *a, int *b)
{
    if (*a > *b)
        return a;

    if (*b > *a)
        return b;
}

int ScoreSquirt(squirt *s)
{
    /*Scoring Considerations:
        store contents
        age
        numbers fired
    */
    int score = 0;

    //store contents
    int i;
    for (i = 0; i < numberTypes; i++)
    {
        score += s->store[i];
    }

    //age
    score += s->age;

    //numbers fired
    score += s->launchCntr;
}

void SortPopulation(squirt *world)
{
    int j = 0, k = population;
    squirt store;

    while (j < k) {

        if (world[j].score < world[j+1].score)
        {
            store = world[j];
            world[j] = world[j+1];
            world[j+1] = store;
            j++;
        }

        if (world[k].score > world[k-1].score)
        {
            store = world[k];
            world[k] = world[k-1];
            world[k-1] = store;
            k--;
        }

        if (world[j].score == world[k=1].score)
        {
            j++;
            k--;
        }

    }
}

void PrintAverage(squirt *world, int gen)
{
    int popSum, avg, p;
    for(p = 0; p < population; p++)
    {
        popSum += world[p].score;
    }

    avg = popSum / population;

    printf("Gen %d: Avg: %d\n", gen, avg);
}

//prints a and b
//prints genome in rows of 100 divided in groups of 10

void PrintGenome(squirt *active)
{
    int g, p;

    for (g = 0; g < genomeSize; g++)
    {
        if (g % 10 == 0 && g % 100 != 0)
        {
            printf(" ");
        }
        else if(g % 100 == 0)
            printf("\n");

        printf("%d", active->genome[g]);
    }

}
