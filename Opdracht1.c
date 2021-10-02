#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "PJ_RPI.h"
#include <time.h>
#include <gpiod.h>
#include <unistd.h>
#include <wiringPi.h>

int Register(con, name, pinstate);
int shtable(con);
int wiringPiRegister(con);

int Register(con, name, pinstate) // libgpiod
{
	int flag = 0;

	while (flag == 0)
	{
		//////////////////////////////////////////////////////// time registration
		time_t rawtime;
		struct tm *timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		char *foo = asctime(timeinfo); // verwijderd de \n\r die je bij de asctime() is.
		foo[strlen(foo) - 1] = 0;

		char state[] = "4";
		char p1[85] = "INSERT INTO subjects( pin, state, time) VALUES ('";
		char p2[] = "',";
		char p3[] = ",'";
		char p4[] = "')";

		sprintf(state, "%d", pinstate); // convert int to char

		strcat(p1, name); // Concatenates p1 met pin
		strcat(p1, p2);	  // Concatenates p1 (p1+pin) met p2
		strcat(p1, state);
		strcat(p1, p3);
		strcat(p1, foo);
		strcat(p1, p4);

		printf("%s\r\n", p1);

		if (mysql_query(con, p1))
		{
			finish_with_error(con);
		}

		flag = 1;
	}
}

int shtable(con)
{
	if (mysql_query(con, "SELECT * FROM subjects"))
	{
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);

	if (result == NULL)
	{
		finish_with_error(con);
	}

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	MYSQL_FIELD *field;

	while ((row = mysql_fetch_row(result)))
	{
		for (int i = 0; i < num_fields; i++)
		{
			if (i == 0)
			{
				while (field = mysql_fetch_field(result))
				{
					printf("%s ", field->name);
				}
				printf("\n");
			}
			printf("%s  ", row[i] ? row[i] : "NULL");
		}
	}

	printf("\n");

	mysql_free_result(result);
	mysql_close(con);
}

int wiringPiRegister(con) // WiringPi
{
	int test = digitalRead(25);
	printf("%d\r\n", test);

	Register(con, "GPIO 26", test);
}

void finish_with_error(MYSQL *con)
{
	fprintf(stderr, "%s\n", mysql_error(con));
	mysql_close(con);
	exit(1);
}

int main(int argc, char **argv)
{
	MYSQL *con = mysql_init(NULL);
	printf("MySQL client version: %s\n", mysql_get_client_info());

	if (con == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		exit(1);
	}

	if (mysql_real_connect(con, "localhost", "webuser", "secretpassword",
						   "globe_bank", 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		mysql_close(con);
		exit(1);
	}

	wiringPiSetup();
	pinMode(25, INPUT);

	while (true)
	{
		wiringPiRegister(con);
	}

	//////////////////////////////////////////////////////// reading
	const char *chipname = "gpiochip0";
	struct gpiod_chip *chip;
	struct gpiod_line *gpio19; // GPIO 19
	struct gpiod_line *gpio21; // GPIO 21
	struct gpiod_line *gpio26; // GPIO 26

	// Open GPIO chip
	chip = gpiod_chip_open_by_name(chipname);

	// Open GPIO lines
	gpio19 = gpiod_chip_get_line(chip, 19);
	gpio21 = gpiod_chip_get_line(chip, 21);
	gpio26 = gpiod_chip_get_line(chip, 26);

	// Open switch line for input
	gpiod_line_request_input(gpio19, "test");
	gpiod_line_request_input(gpio21, "test");
	gpiod_line_request_input(gpio26, "test");

	int val = 0, val1 = 0, bal = 0, bal1 = 0, dal = 0, dal1 = 0;

	while (true)
	{
		val = gpiod_line_get_value(gpio21);
		bal = gpiod_line_get_value(gpio19);
		dal = gpiod_line_get_value(gpio26);

		if (val != val1)
		{
			Register(con, "GPIO 21", val);
		}
		if (dal != dal1)
		{
			Register(con, "GPIO 26", dal);
		}
		if (bal != bal1)
		{
			Register(con, "GPIO 19", bal);
			break; // exits while loop
		}

		val1 = val;
		dal1 = dal;
		bal1 = bal;
	}

	shtable(con);

	// Release lines and chip
	gpiod_line_release(gpio19);
	gpiod_line_release(gpio21);
	gpiod_line_release(gpio26);

	exit(0);
}