/* bankAccount.h simulates a simple bank account
 *
 * Joel Adams, Calvin College, Fall 2013.
 */


// Shared Variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
double bankAccountBalance[5];

// add amount to bankAccountBalance
void deposit(double amount, int numero);
// subtract amount from bankAccountBalance
void withdraw(double amount,int numero);
double transferencia(int origem, int destino, double valor);
double saldo(int conta);
void cleanup();
