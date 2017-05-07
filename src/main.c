#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>


typedef struct buffer
{
  int buffer[50];
  int amount;
  int bigger_generated;
  int smaller_generated;
} Buffer;

typedef struct c_args
{
  Buffer *buffer;
  char thread_id;
} C_args;

int random_number()
{
  long random = rand()%RAND_MAX - rand()%RAND_MAX;

  return random;
}

void sleep_ms(int ms)
{
  usleep(ms*1000);
}

void initialize_buffer(Buffer *buffer)
{
  buffer->amount = 0;
  buffer->bigger_generated = 0;
  buffer->smaller_generated = 0;

  int i;
  for(i=0; i<50; i++)
  {
    buffer->buffer[i] = 0;
  }
}

void write_to_buffer(Buffer *buffer, int number)
{
  for(unsigned int i=0; i<50; i++)
  {
    if(buffer->buffer[i] == 0)
    {
      buffer->buffer[i] = number;
      buffer->amount++;

      break;
    }
  }
}

long read_from_buffer(Buffer *buffer)
{
  for(unsigned int i=0; i<50; i++)
  {
    if(buffer->buffer[i] != 0)
    {
      long aux = buffer->buffer[i];
      buffer->buffer[i] = 0; // Excluding from buffer
      buffer->amount--;
      return aux;
    }
  }
  // #TODO Caso não haja nada a ler, o que fazer??
  return -1;
}

// Thread produtora
void *producerThread(void *arg)
{
  Buffer *buffer = (Buffer *)arg;
  while(1)
  {
    long number = random_number();

    write_to_buffer(buffer, number);
    sleep_ms(500);

    // Escrever no log
    printf("[producao]: Numero gerado: %ld\n", number);
  }
  return NULL;
}


// void *consumerThread(Buffer *buffer, char thread_id)
void *consumerThread(void *arg)
{
  C_args *args = (C_args *)arg;

  long number = read_from_buffer(args->buffer);

  // Calcular maior
  // bigger_number_calculation(buffer, number);
  // Calcular menor
  // Escrever no log
  printf("[consumo %c]: Numero lido: %ld\n", args->thread_id, number);
  sleep_ms(150);

  return NULL;
}



int main(int argc, char **argv)
{
  srand((unsigned)time(NULL));
  Buffer buffer;
  pthread_t threads[3];
  initialize_buffer(&buffer);
  char *output_string = argv[1];
  C_args args1;
  C_args args2;
  args1.buffer = &buffer;
  args1.thread_id = 'a';
  args2.buffer = &buffer;
  args2.thread_id = 'b';
  FILE *output_file = fopen(output_string, "a");

  if (output_file == NULL)
  {
    printf("ERROR: File could not be opened!\n");
    exit(-1);
  }

    pthread_create(&threads[0], NULL, producerThread, (void *)&buffer);
    pthread_create(&threads[1], NULL, consumerThread, (void *)&args1);
    pthread_create(&threads[2], NULL, consumerThread, (void *)&args2);
    sleep_ms(2000);

  return 0;
}
