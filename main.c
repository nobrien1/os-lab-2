#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// Define constants
#define NUM_INGREDIENTS 3
#define MAX_SLEEP 200

// Enum to represent ingredients
enum Ingredient
{
    TOBACCO,
    PAPER,
    MATCH
};

// Semaphore variables
sem_t agentSem;
sem_t tobacco, paper, match;
sem_t tobaccoSem, paperSem, matchSem;
sem_t mutex;

// Bools for each ingredient
bool isTobacco = false, isPaper = false, isMatch = false;

// Function to sleep thread for random amount of time: 0 <= t < max_ms
void randomSleep(int max_ms)
{
    usleep((rand() % max_ms) * 1000);
}

// Agent function thread that takes in two ingredients
void *agentTobaccoPaper(void *arg)
{
    for (int i = 0; i < 6; i++)
    {
        randomSleep(MAX_SLEEP);
        sem_wait(&agentSem);
        printf("\nAgent puts tobacco and paper on the table #%d\n", i + 1);
        sem_post(&tobacco);
        sem_post(&paper);

        int tobaccoValue, paperValue, matchValue, mutexValue;
        sem_getvalue(&tobacco, &tobaccoValue);
        sem_getvalue(&paper, &paperValue);
        sem_getvalue(&match, &matchValue);
        sem_getvalue(&mutex, &mutexValue);
    }
    pthread_exit(NULL);
}

// Agent function thread that takes in two ingredients
void *agentPaperMatch(void *arg)
{
    for (int i = 0; i < 6; i++)
    {
        randomSleep(MAX_SLEEP);
        sem_wait(&agentSem);
        printf("\nAgent puts paper and match on the table #%d\n", i + 1);
        sem_post(&paper);
        sem_post(&match);

        int tobaccoValue, paperValue, matchValue, mutexValue;
        sem_getvalue(&tobacco, &tobaccoValue);
        sem_getvalue(&paper, &paperValue);
        sem_getvalue(&match, &matchValue);
        sem_getvalue(&mutex, &mutexValue);
    }
    pthread_exit(NULL);
}

// Agent function thread that takes in two ingredients
void *agentMatchTobacco(void *arg)
{
    for (int i = 0; i < 6; i++)
    {
        randomSleep(MAX_SLEEP);
        sem_wait(&agentSem);
        printf("\nAgent puts match and tobacco on the table #%d\n", i + 1);
        sem_post(&match);
        sem_post(&tobacco);

        int tobaccoValue, paperValue, matchValue, mutexValue;
        sem_getvalue(&tobacco, &tobaccoValue);
        sem_getvalue(&paper, &paperValue);
        sem_getvalue(&match, &matchValue);
        sem_getvalue(&mutex, &mutexValue);
    }
    pthread_exit(NULL);
}

// Pusher function thread
void *pusherTobacco(void *arg)
{
    for (int i = 0; i < 12; i++)
    {
        sem_wait(&tobacco);
        sem_wait(&mutex);
        if (isPaper)
        {
            isPaper = 0;
            sem_post(&matchSem);
        }
        else if (isMatch)
        {
            isMatch = 0;
            sem_post(&paperSem);
        }
        else
        {
            isTobacco = 1;
        }
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}

void *pusherPaper(void *arg)
{
    for (int i = 0; i < 12; i++)
    {
        sem_wait(&paper);
        sem_wait(&mutex);
        if (isTobacco)
        {
            isTobacco = 0;
            sem_post(&matchSem);
        }
        else if (isMatch)
        {
            isMatch = 0;
            sem_post(&tobaccoSem);
        }
        else
        {
            isPaper = 1;
        }
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}

void *pusherMatch(void *arg)
{
    for (int i = 0; i < 12; i++)
    {
        sem_wait(&match);
        sem_wait(&mutex);
        if (isTobacco)
        {
            isTobacco = 0;
            sem_post(&paperSem);
        }
        else if (isPaper)
        {
            isPaper = 0;
            sem_post(&tobaccoSem);
        }
        else
        {
            isMatch = 1;
        }
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}

// Smoker function thread that takes in an ingredient
void *smoker(void *arg)
{
    sem_t *ingredientSem;
    char *ingredientName;
    switch ((enum Ingredient)arg)
    {
    case TOBACCO:
        ingredientSem = &tobaccoSem;
        ingredientName = "tobacco";
        break;
    case PAPER:
        ingredientSem = &paperSem;
        ingredientName = "paper";
        break;
    case MATCH:
        ingredientSem = &matchSem;
        ingredientName = "match";
        break;
    }

    for (int i = 0; i < 3; i++)
    {
        sem_wait(ingredientSem);
        // Make and smoke cigarette
        printf("Smoker with %s makes and smokes cigarette #%d\n", ingredientName, i + 1);
        randomSleep(50);
        sem_post(&agentSem);
    }

    printf("Smoker with %s is done smoking\n", ingredientName);
    pthread_exit(NULL);
}

int main()
{
    // Set random seed for rand()
    srand(time(NULL));

    // Initialize semaphores
    sem_init(&agentSem, 0, 1);
    sem_init(&tobacco, 0, 0);
    sem_init(&paper, 0, 0);
    sem_init(&match, 0, 0);
    sem_init(&tobaccoSem, 0, 0);
    sem_init(&paperSem, 0, 0);
    sem_init(&matchSem, 0, 0);
    sem_init(&mutex, 0, 1);

    // Create agent, pusher, and smoker threads
    pthread_t agents[3], pushers[3], smokers[3];

    sem_t agent1Ingredients[2] = {TOBACCO, PAPER};
    sem_t agent2Ingredients[2] = {PAPER, MATCH};
    sem_t agent3Ingredients[2] = {TOBACCO, MATCH};

    pthread_create(&agents[0], NULL, agentTobaccoPaper, NULL);
    pthread_create(&agents[1], NULL, agentPaperMatch, NULL);
    pthread_create(&agents[2], NULL, agentMatchTobacco, NULL);

    pthread_create(&pushers[0], NULL, pusherMatch, NULL);
    pthread_create(&pushers[1], NULL, pusherTobacco, NULL);
    pthread_create(&pushers[2], NULL, pusherPaper, NULL);

    pthread_create(&smokers[0], NULL, smoker, (void *)TOBACCO);
    pthread_create(&smokers[1], NULL, smoker, (void *)PAPER);
    pthread_create(&smokers[2], NULL, smoker, (void *)MATCH);
    pthread_create(&smokers[0], NULL, smoker, (void *)TOBACCO);
    pthread_create(&smokers[1], NULL, smoker, (void *)PAPER);
    pthread_create(&smokers[2], NULL, smoker, (void *)MATCH);

    // Wait for all threads to finish
    for (int i = 0; i < 3; i++)
    {
        pthread_join(agents[i], NULL);
        pthread_join(pushers[i], NULL);
        pthread_join(smokers[i], NULL);
    }

    // Destroy semaphores
    sem_destroy(&agentSem);
    sem_destroy(&tobacco);
    sem_destroy(&paper);
    sem_destroy(&match);
    sem_destroy(&tobaccoSem);
    sem_destroy(&paperSem);
    sem_destroy(&matchSem);
    sem_destroy(&mutex);

    return 0;
}
