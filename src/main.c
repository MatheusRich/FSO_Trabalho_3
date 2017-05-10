#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#define TRUE 1
#define FALSE 0
#define MESSAGE_MAX_SIZE 250
#define BUFFER_MAX_SIZE 50

int bigger_ocupation = 0;
int running = TRUE;
long bigger_generated = 0;
long smaller_generated = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;
pthread_t threads[3];

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
  remove("doc/file.txt");
}

int random_number()
{
  long random = rand()%RAND_MAX;

  return random%2 == 0 ? random:-random;
}

void sleep_ms(int ms)
{
  usleep(ms*1000);
}

void signal_handler(int sig)
{
  signal(sig, SIG_IGN);

  printf("\n\n[aviso]: Termino solicitado. Aguardando threads...");
  running = FALSE;
  pthread_cancel(threads[0]);
  pthread_cancel(threads[1]);
  pthread_cancel(threads[2]);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&can_consume);
  pthread_cond_destroy(&can_produce);
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
    buffer->buffer[i] = -666;
  }
}

long read_from_buffer(Buffer *buffer, char id)
{
  // printf(">%c< LENDO o %d\n", id, (buffer->amount)-1);
  long aux = buffer->buffer[(buffer->amount)-1];
  buffer->amount--;
  return aux;
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
  // printf("produzindo no %d\n", buffer->amount);
  buffer->buffer[buffer->amount] = number;
  buffer->amount++;
  bigger_ocupation_calculation(buffer, buffer->amount);
}

void log_message(Buffer *buffer, char *message)
{
  FILE *output_file = fopen(buffer->output_string, "a");
  // printf("%s\n",buffer->output_string );

  if(output_file == NULL)
  {
    printf("LOG_ERROR: FILE \"%s\" COULD NOT BE OPENED!\n", buffer->output_string);
    exit(0);
  }

  fprintf(output_file, "%s", message);
  fclose(output_file);
}

void *producerThread(void *arg)
{
  Buffer *buffer = (Buffer *)arg;
  int old_cancel_state;

  while(running)
  {

    // printf("Tenta MUTEX P: ON\n");
    pthread_mutex_lock(&mutex);
    // printf("MUTEX P: ON\n");

    if(buffer->amount == BUFFER_MAX_SIZE)
    {
      // printf("Esperando pra produzir\n");
      pthread_cond_wait(&can_produce, &mutex);
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancel_state);

    // printf("Entrar produção\n");
    long number = random_number();

    write_to_buffer(buffer, number);

    char message[MESSAGE_MAX_SIZE + 1];
    snprintf(message, MESSAGE_MAX_SIZE, "[producao]: Numero gerado: %ld\n", number);
    log_message(buffer, message);

    pthread_cond_signal(&can_consume);
    // printf("MUTEX P: OFF\n");
    pthread_mutex_unlock(&mutex);

    pthread_setcancelstate(old_cancel_state, NULL);
    pthread_testcancel();

    sleep_ms(100);
  }
  return NULL;
}

void *consumerThread(void *arg)
{
  C_args *args = (C_args *)arg;
  int old_cancel_state;

  while(running)
  {
    // printf(">%c< Tenta MUTEX C\n", args->thread_id);
    pthread_mutex_lock(&mutex);
    // printf("MUTEX C: ON\n");

    while(args->buffer->amount == 0)
    {
      // printf(">%c< Esperando pra consumir\n", args->thread_id);
      pthread_cond_wait(&can_consume, &mutex);
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_cancel_state);

    long number = 0;

    // printf(">%c< Entrar consumo\n", args->thread_id);
    number = read_from_buffer(args->buffer, args->thread_id);

    bigger_number_calculation(args->buffer, number);
    smaller_number_calculation(args->buffer, number);

    char message[MESSAGE_MAX_SIZE + 1];
    snprintf(message, MESSAGE_MAX_SIZE,"[consumo %c]: Numero lido: %ld\n", args->thread_id, number);
    log_message(args->buffer, message);

    pthread_cond_signal(&can_produce);
    // printf("MUTEX C: OFF\n");
    pthread_mutex_unlock(&mutex);

    pthread_setcancelstate(old_cancel_state, NULL);
    pthread_testcancel();

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

  //
  signal(SIGINT, signal_handler);

  Buffer buffer;
  C_args args1;
  C_args args2;


  initialize_buffer(&buffer);
  args1.buffer = &buffer;
  args1.thread_id = 'a';
  args2.buffer = &buffer;
  args2.thread_id = 'b';

  char temp_string[MESSAGE_MAX_SIZE] = "";
  strcpy(temp_string, argv[1]);
  buffer.output_string = temp_string;

  if (buffer.output_string == NULL)
  {
    printf("ERROR: Input a file!\n");

    return 0;
  }

  pthread_create(&threads[0], NULL, producerThread, (void *)&buffer);
  pthread_create(&threads[1], NULL, consumerThread, (void *)&args1);
  pthread_create(&threads[2], NULL, consumerThread, (void *)&args2);

  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);

  char final_message[MESSAGE_MAX_SIZE] = "";
  snprintf(final_message, MESSAGE_MAX_SIZE, "\n[aviso]: Maior numero gerado: %ld\n[aviso]: Menor numero gerado: %ld\n[aviso]: Maior ocupacao de buffer: %d\n[aviso]: Aplicacao encerrada.\n", bigger_generated, smaller_generated, bigger_ocupation);
  printf("%s", final_message);
  log_message(&buffer, final_message);

  return 0;
}
