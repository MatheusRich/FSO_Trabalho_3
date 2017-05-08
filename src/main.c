#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#define TRUE 1
#define FALSE 0
#define MESSAGE_MAX_SIZE 100

// char *output_string;
int bigger_ocupation = 0;
long bigger_generated = 0;
long smaller_generated = 0;

typedef struct buffer
{
  int buffer[50];
  int amount;
  int first_bigger_number;
  int first_smaller_number;
  int first_ocupation;
  char *output_string;
} Buffer;

typedef struct c_args
{
  Buffer *buffer;
  char thread_id;
} C_args;

void clean_output()
{
  remove("file.txt");
}

void cancel_handler(int sig)
{
  signal(sig, SIG_IGN);

  exit(2);
}

int random_number()
{
  long random = rand()%RAND_MAX - rand()%RAND_MAX;

  return random;
}

void sleep_ms(int ms)
{
  usleep(ms*1000);
}

void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     printf("\n\n[aviso]: Termino solicitado. Aguardando threads...\n");
     sleep_ms(200);
     printf("[aviso]: Maior numero gerado: %ld\n", bigger_generated);
     printf("[aviso]: Menor numero gerado: %ld\n", smaller_generated);
     printf("[aviso]: Maior ocupacao de buffer: %d\n", bigger_ocupation);
     printf("[aviso]: Aplicacao encerrada.\n");

     exit(0);
}

void initialize_buffer(Buffer *buffer)
{
  buffer->amount = 0;
  buffer->first_bigger_number = TRUE;
  buffer->first_smaller_number = TRUE;
  buffer->first_ocupation = TRUE;
  buffer->output_string = NULL;

  int i;
  for(i=0; i<50; i++)
  {
    buffer->buffer[i] = 0;
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
  // return read_from_buffer(buffer);
}

void bigger_number_calculation(Buffer *buffer, long number)
{
  if(buffer->first_bigger_number || number > bigger_generated)
  {
    bigger_generated = number;
    buffer->first_bigger_number = FALSE;
  }
}

void bigger_ocupation_calculation(Buffer *buffer, long number)
{
  if(buffer->first_ocupation || number > bigger_ocupation)
  {
    bigger_ocupation = number;
    buffer->first_ocupation = FALSE;
  }
}

void smaller_number_calculation(Buffer *buffer, long number)
{
  if(buffer->first_smaller_number || number < smaller_generated)
  {
    smaller_generated= number;
    buffer->first_smaller_number = FALSE;
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
      bigger_ocupation_calculation(buffer, buffer->amount);

      break;
    }
  }
}

void log_message(Buffer *buffer, char *message)
{
  FILE *output_file = fopen(buffer->output_string, "a");

  if(output_file == NULL)
  {
    printf("ERROR: FILE \"%s\" COULD NOT BE OPENED!\n", buffer->output_string);
    exit(-1);
  }

  fprintf(output_file, "%s", message);
  fclose(output_file);
}

// Thread produtora
void *producerThread(void *arg)
{
  Buffer *buffer = (Buffer *)arg;

  while(1)
  {
    long number = random_number();
    write_to_buffer(buffer, number);

    char message[MESSAGE_MAX_SIZE + 1];
    snprintf(message, MESSAGE_MAX_SIZE, "[producao]: Numero gerado: %ld\n", number);
    log_message(buffer, message);

    sleep_ms(100);
  }

  return NULL;
}

void *consumerThread(void *arg)
{
  C_args *args = (C_args *)arg;

  long number = 0;

  while (1)
  {
    do
    {
      number = read_from_buffer(args->buffer);
    } while(number == -1);

    bigger_number_calculation(args->buffer, number);
    smaller_number_calculation(args->buffer, number);

    char message[MESSAGE_MAX_SIZE + 1];
    snprintf(message, MESSAGE_MAX_SIZE,"[consumo %c]: Numero lido: %ld\n", args->thread_id, number);
    log_message(args->buffer, message);

    sleep_ms(150);
  }

  return NULL;
}



int main(int argc, char **argv)
{
  // Seeding random_number()
  srand((unsigned)time(NULL));

  // Cleaning standard output file
  clean_output();

  signal(SIGINT, INThandler);
  Buffer buffer;
  C_args args1;
  C_args args2;
  pthread_t threads[3];

  initialize_buffer(&buffer);
  args1.buffer = &buffer;
  args1.thread_id = 'a';
  args2.buffer = &buffer;
  args2.thread_id = 'b';

  buffer.output_string = argv[1];

  if (buffer.output_string == NULL)
  {
    printf("ERROR: Input a file!\n");

    return 0;
  }

  pthread_create(&threads[0], NULL, producerThread, (void *)&buffer);
  sleep_ms(20);
  pthread_create(&threads[1], NULL, consumerThread, (void *)&args1);
  pthread_create(&threads[2], NULL, consumerThread, (void *)&args2);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);

  return 0;
}
