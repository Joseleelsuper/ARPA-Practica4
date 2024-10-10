#include <mpi.h>
#include <iostream>

using namespace std;

constexpr int TAM_FILA = 4;         // Filas
constexpr int TAM_COLUMNA = 3;      // Columnas
constexpr int RANK_MASTER = 0;      // Rango del proceso maestro
constexpr int NUM_DIMENSIONES = 2;  // N�mero de dimensiones de la topolog�a cartesiana
constexpr int NUM_DATOS = 1;        // N�mero de datos a enviar

/*
    Funci�n que se necarga de generar una matriz de tama�o N x N con valores aleatorios.

    @param matrix: matriz que almacena la matriz.
*/
void generateMatrix(int matrix[TAM_FILA][TAM_COLUMNA]);
/*
    Funci�n que se encarga de imprimir una matriz de tama�o N x N.

    @param matrix: matriz que almacena la matriz.
*/
void printMatrix(int matrix[TAM_FILA][TAM_COLUMNA]);
/*
    Funci�n que se encarga de imprimir una l�nea horizontal de la matriz.
*/
void printLine();

/*
    Funci�n principal.
*/
int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Verificar que el n�mero de procesos sea igual al tama�o de la matriz
    if (size != TAM_FILA * TAM_COLUMNA) {
        // Imprimir un mensaje de error en el proceso 0, evitando que los dem�s procesos muestren el mismo mensaje
        if (rank == RANK_MASTER) {
            printf("El n�mero de procesos debe ser igual al tama�o de la matriz (%d).\n", TAM_FILA * TAM_COLUMNA);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Crear la topolog�a cartesiana
    int dims[2] = { TAM_FILA, TAM_COLUMNA };
    int periods[2] = { 0, 0 };  // No peri�dica
    int reorder = 1;            // Permitir al comunicador reordenar los procesos
    MPI_Comm cart_comm;
    MPI_Cart_create(MPI_COMM_WORLD, NUM_DIMENSIONES, dims, periods, reorder, &cart_comm);

    // Obtener las coordenadas del proceso
    int coords[2];
    MPI_Cart_coords(cart_comm, rank, NUM_DIMENSIONES, coords);

    // Inicializar las matrices A, B y C
    int A[TAM_FILA][TAM_COLUMNA], B[TAM_FILA][TAM_COLUMNA], C[TAM_FILA][TAM_COLUMNA] = { 0 };

    // Generar las matrices A y B en el proceso 0 y mostrarlas
    if (rank == RANK_MASTER) {
        srand(time(NULL)); // Inicializar la semilla para generar n�meros aleatorios
        generateMatrix(A);
        generateMatrix(B);
        printf("Matriz A:\n");
        printMatrix(A);
        printf("Matriz B:\n");
        printMatrix(B);
    }
    // Medir el tiempo de inicio
    double startTime = MPI_Wtime();

    // Distribuir las matrices A y B a todos los procesos
    MPI_Bcast(A, TAM_FILA * TAM_COLUMNA, MPI_INT, RANK_MASTER, cart_comm);
    MPI_Bcast(B, TAM_FILA * TAM_COLUMNA, MPI_INT, RANK_MASTER, cart_comm);

    // Cada proceso suma su elemento correspondiente
    int row = coords[0];
    int col = coords[1];
    C[row][col] = A[row][col] + B[row][col];

    // Recolectar resultados en la matriz C en el proceso maestro
    MPI_Gather(&C[row][col], NUM_DATOS, MPI_INT, C, NUM_DATOS, MPI_INT, RANK_MASTER, cart_comm);

    if (rank == RANK_MASTER) {
        // Medir el tiempo de finalizaci�n
        double endTime = MPI_Wtime();
        printf("Matriz C (resultado A+B):\n");
        printMatrix(C);
        printf("Tiempo de ejecuci�n: %f segundos\n", endTime - startTime);
    }

    MPI_Finalize();
    return 0;
}

void generateMatrix(int matrix[TAM_FILA][TAM_COLUMNA]) {
    for (int i = 0; i < TAM_FILA; ++i) {
        for (int j = 0; j < TAM_COLUMNA; ++j) {
            matrix[i][j] = rand() % 10; // N�meros aleatorios entre 0 y 9
        }
    }
}

void printLine() {
    printf("+");
    for (int j = 0; j < TAM_COLUMNA - 1; ++j) {
        printf("-----");
    }
    printf("----+\n");
}

void printMatrix(int matrix[TAM_FILA][TAM_COLUMNA]) {
    // Imprimir la l�nea superior
    printLine();

    for (int i = 0; i < TAM_FILA; ++i) {
        // Imprimir los valores de la fila
        printf("|");
        for (int j = 0; j < TAM_COLUMNA; ++j) {
            printf(" %2d |", matrix[i][j]);
        }
        printf("\n");

        // Imprimir las l�neas intermedias e inferior de la fila
        printLine();
    }
}
