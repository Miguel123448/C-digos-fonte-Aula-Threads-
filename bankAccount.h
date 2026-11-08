/* bankAccount.h simulates a simple bank account
 *
 * Joel Adams, Calvin College, Fall 2013.
 */


// Shared Variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
double bankAccountBalance[5];

// add amount to bankAccountBalance
void deposit(double amount, int numero) {
  if(amount <=0){
    return;
  }
  pthread_mutex_lock( &lock );
  bankAccountBalance[numero] += amount;
  pthread_mutex_unlock( &lock );
}

// subtract amount from bankAccountBalance
void withdraw(double amount,int numero) {

    pthread_mutex_lock(&lock);

    if (amount > 0 && bankAccountBalance[numero] >= amount) {
        bankAccountBalance[numero] -= amount;
        pthread_mutex_unlock(&lock);
        printf("Saque realizado!\n");
    } else {
        pthread_mutex_unlock(&lock);
        printf("Saldo insuficiente ou valor invalido!\n");
    }
}

void cleanup() {
   pthread_mutex_destroy(&lock);
}

