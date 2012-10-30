#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

int main(int argc, char *argv[]) {
	int rank, size;
	int round;
	sscanf(argv[1], "%d", &round);
	char numbb[3][100];
	char mssg[3][100];
	double a;
	double G = 6.67e-11;
	double b;
	double ttt = 2.0;
	double vx = 0, vy = 0; /*body's x-speed and y-speed*/
	char tnumber[100];
	double number[100];
	char tmsg[100];
	double msg[100];
	int i;
	int j = 0;
	int k;
	int m;
	double fx, fy, tt, ds, ff;
	double recno[100]; /*this variable is used to receive the number from other n-1 body and then send it to the next one*/

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (rank == 0) {
		/*--------------root node read file and send initial states to others---------------------*/
		FILE *fp;

		fp = fopen(argv[2], "r+"); /*"d:\\file\\number.txt"*/
		if (fp == NULL ) {

			return -1;

		}
		fgets(tnumber, 30, fp);

		sscanf(tnumber, "%s%s%s", numbb[0], numbb[1], numbb[2]);
		number[0] = strtod(numbb[0], NULL );
		number[1] = strtod(numbb[1], NULL );
		number[2] = strtod(numbb[2], NULL );
		while (fgets(tmsg, 30, fp) != NULL ) {
			/*printf("I get %s\n here", tmsg);*/
			sscanf(tmsg, "%s%s%s", mssg[0], mssg[1], mssg[2]);
			msg[0] = strtod(mssg[0], NULL );
			msg[1] = strtod(mssg[1], NULL );
			msg[2] = strtod(mssg[2], NULL );
			MPI_Send(msg, 3, MPI_DOUBLE, rank + 1, 998, MPI_COMM_WORLD );
			/*printf("I send %lf here \n", msg[0]);*/
		}
		fclose(fp);
		/*-----------------------round 1 start------------------------------------------*/
		for (m = 0; m < round; m++) {
			MPI_Send(number, 3, MPI_DOUBLE, rank + 1, m, MPI_COMM_WORLD );
			/*printf("I am %d and my number is %lf %lf %lf\n", rank, number[0],
					number[1], number[2]);*/
			fx = 0;
			fy = 0;
			for (i = 0; i < size - 2; i++) {

				MPI_Recv(recno, 3, MPI_DOUBLE, size - 1, m, MPI_COMM_WORLD,
						&status);
				tt = recno[0] * number[0] * G;
				ds = (recno[1] - number[1]) * (recno[1] - number[1])
						+ (recno[2] - number[2]) * (recno[2] - number[2]);
				ff = tt / ds;
				fx = fx + ff * (recno[1] - number[1]) / sqrt(ds);
				fy = fy + ff * (recno[2] - number[2]) / sqrt(ds);

				/*printf("I am %d and I receive%lf %lf %lf\n", rank, recno[0],recno[1], recno[2]);*/

				MPI_Send(recno, 3, MPI_DOUBLE, rank + 1, m, MPI_COMM_WORLD );
			}
			MPI_Recv(recno, 3, MPI_DOUBLE, size - 1, m, MPI_COMM_WORLD,
					&status);
			tt = recno[0] * number[0] * G;
			ds = (recno[1] - number[1]) * (recno[1] - number[1])
					+ (recno[2] - number[2]) * (recno[2] - number[2]);
			ff = tt / ds;
			fx = fx + ff * (recno[1] - number[1]) / sqrt(ds);
			fy = fy + ff * (recno[2] - number[2]) / sqrt(ds);
			/*printf("I am %d and I receive %lf %lf %lf\n", rank, recno[0], recno[1],recno[2]);*/
			/*now given the fx and fy,we can update velocity and position*/
			vx = vx + fx * ttt / number[0];
			vy = vy + fy * ttt / number[0];
			number[1] = number[1] + vx * ttt;
			number[2] = number[2] + vy * ttt;

		}
		/*---------------------root node collect file and print it out-----------------------------*/
		printf(
				"after %d round running, the result of body 0 is: %lf %lf %lf\n", round,
				number[0], number[1], number[2]);

		for (i = 0; i < size - 1; i++) {
			MPI_Recv(recno, 3, MPI_DOUBLE, rank + 1, 999, MPI_COMM_WORLD,
					&status);
			printf(
					"after %d round running, the result of body %d is: %lf %lf %lf\n",round,i+1,
					recno[0], recno[1], recno[2]);
		}

	} else if (rank == size - 1) {
		/*----------------------------round 0 start--------------------------------*/
		MPI_Recv(number, 3, MPI_DOUBLE, rank - 1, 998, MPI_COMM_WORLD, &status);
		/*printf("I am %d and receive number %s\n", rank,number );*/
		/*------------------------------round 1 start----------------------------------*/
		for (m = 0; m < round; m++) {
			MPI_Send(number, 3, MPI_DOUBLE, 0, m, MPI_COMM_WORLD );
			/*printf("I am %d and my number is %lf %lf %lf\n", rank, number[0],
					number[1], number[2]);*/
			fx = 0;
			fy = 0;
			for (i = 0; i < size - 2; i++) {

				MPI_Recv(recno, 3, MPI_DOUBLE, rank - 1, m, MPI_COMM_WORLD,
						&status);
				tt = recno[0] * number[0] * G;
				ds = (recno[1] - number[1]) * (recno[1] - number[1])
						+ (recno[2] - number[2]) * (recno[2] - number[2]);
				ff = tt / ds;
				fx = fx + ff * (recno[1] - number[1]) / sqrt(ds);
				fy = fy + ff * (recno[2] - number[2]) / sqrt(ds);

				/*printf("I am %d and I receive%lf %lf %lf\n", rank, recno[0],recno[1], recno[2]);*/

				MPI_Send(recno, 3, MPI_DOUBLE, 0, m, MPI_COMM_WORLD );
			}
			MPI_Recv(recno, 3, MPI_DOUBLE, rank - 1, m, MPI_COMM_WORLD,
					&status);
			tt = recno[0] * number[0] * G;
			ds = (recno[1] - number[1]) * (recno[1] - number[1])
					+ (recno[2] - number[2]) * (recno[2] - number[2]);
			ff = tt / ds;
			fx = fx + ff * (recno[1] - number[1]) / sqrt(ds);
			fy = fy + ff * (recno[2] - number[2]) / sqrt(ds);
			/*printf("I am %d and I receive%lf %lf %lf\n", rank, recno[0], recno[1],recno[2]);*/
			vx = vx + fx * ttt / number[0];
			vy = vy + fy * ttt / number[0];
			number[1] = number[1] + vx * ttt;
			number[2] = number[2] + vy * ttt;
		}
		/*---------------here the last processor start to send its number to others to print-----*/
		MPI_Send(number, 3, MPI_DOUBLE, rank - 1, 999, MPI_COMM_WORLD );

	} else {
		/*-----------------------round 0 --------------------------------------start*/
		MPI_Recv(number, 3, MPI_DOUBLE, rank - 1, 998, MPI_COMM_WORLD, &status);
		int temp = size - rank - 1;
		while (temp > 0) {
			MPI_Recv(msg, 3, MPI_DOUBLE, rank - 1, 998, MPI_COMM_WORLD,
					&status);

			/*printf("I am %d and send message %s\n", rank,msg);*/
			MPI_Send(msg, 3, MPI_DOUBLE, rank + 1, 998, MPI_COMM_WORLD );
			temp = temp - 1;
		}
		/*----------------------------round 1 start--------------------------------------*/
		for (m = 0; m < round; m++) {
			MPI_Send(number, 3, MPI_DOUBLE, rank + 1, m, MPI_COMM_WORLD );
			/*printf("I am %d and my number is %lf %lf %lf\n", rank, number[0],
					number[1], number[2]);*/

			fx = 0;
			fy = 0;
			for (i = 0; i < size - 2; i++) {

				MPI_Recv(recno, 3, MPI_DOUBLE, rank - 1, m, MPI_COMM_WORLD,
						&status);
				tt = recno[0] * number[0] * G;
				ds = (recno[1] - number[1]) * (recno[1] - number[1])
						+ (recno[2] - number[2]) * (recno[2] - number[2]);
				ff = tt / ds;
				fx = fx + ff * (recno[1] - number[1]) / sqrt(ds);
				fy = fy + ff * (recno[2] - number[2]) / sqrt(ds);
            			/*printf("I am %d and I receive%lf %lf %lf\n", rank, recno[0],recno[1], recno[2]);*/
				MPI_Send(recno, 3, MPI_DOUBLE, rank + 1, m, MPI_COMM_WORLD );
			}
			MPI_Recv(recno, 3, MPI_DOUBLE, rank - 1, m, MPI_COMM_WORLD,
					&status);
			tt = recno[0] * number[0] * G;
			ds = (recno[1] - number[1]) * (recno[1] - number[1])
					+ (recno[2] - number[2]) * (recno[2] - number[2]);
			ff = tt / ds;
			fx = fx + ff * (recno[1] - number[1]) / sqrt(ds);
			fy = fy + ff * (recno[2] - number[2]) / sqrt(ds);
			/*printf("I am %d and I receive%lf %lf %lf\n", rank, recno[0], recno[1],recno[2]);*/
			vx = vx + fx * ttt / number[0];
			vy = vy + fy * ttt / number[0];
			number[1] = number[1] + vx * ttt;
			number[2] = number[2] + vy * ttt;
		}
		/*------here processor 1 to N-2 start to send number to processor 0 to print out the number----*/
		MPI_Send(number, 3, MPI_DOUBLE, rank - 1, 999, MPI_COMM_WORLD );
		for (i = 0; i < size - rank - 1; i++) {
			MPI_Recv(recno, 3, MPI_DOUBLE, rank + 1, 999, MPI_COMM_WORLD,
					&status);
			MPI_Send(recno, 3, MPI_DOUBLE, rank - 1, 999, MPI_COMM_WORLD );
		}

	}

	MPI_Finalize();
	return 0;
}

