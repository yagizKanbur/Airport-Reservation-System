/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>
#include "stdafx.h"
#include <conio.h>

/*
Include directly the different
headers from cppconn/ and mysql_driver.h + mysql_util.h
(and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

void createTables(sql::Statement *stmt);
void mainLoop(sql::PreparedStatement *, sql::Statement *, sql::Connection *, sql::ResultSet *);
void menu(sql::PreparedStatement *, sql::Statement *, sql::Connection *, sql::ResultSet *);
int welcomeMessage();
void menuInterface();
void newFlight(sql::PreparedStatement *, sql::Statement *, sql::Connection *);
void addFlight(sql::PreparedStatement*, char*, char*, int, int, char*);
void sellTicket(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *);
void listFlights(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *, int, int, char *, int);
int checkCapacity(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *, char *);
bool checkTCKN(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *, char*);
void addSoldTicket(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *, int , int , char* , char *, char *, char *);
void addPassenger(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *, char* );
void passengerQuery(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *);
void cancelFlight(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *, sql::Statement *);
void cancelTicket(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *);
void flightQuery(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *);
void listFlightOccupancyRate(sql::PreparedStatement *, sql::ResultSet *, sql::Connection *);
void addAirplane(sql::PreparedStatement *, sql::Connection *);
void addAirplanetoDB(sql::PreparedStatement*, int, char*, char*);
void errorMessages(sql::SQLException &/*, sql::PreparedStatement *, sql::Statement *, sql::Connection *, sql::ResultSet * */);

int main(void)
{
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "1234");
		/* Connect to the MySQL test database */
		con->setSchema("test");
		stmt = con->createStatement();
		createTables(stmt);
		mainLoop(pstmt, stmt, con, res);

		delete res;
		delete pstmt;
		delete con;
		delete stmt;
	}
	catch (sql::SQLException &e) {
		errorMessages(e);
	}
	return EXIT_SUCCESS;
}


// Returns nothing. Creates necessary tables if tables not exist.
void createTables(sql::Statement *stmt) {
	//stmt->execute("SET FOREIGN_KEY_CHECKS = 0");
	stmt->execute("CREATE TABLE IF NOT EXISTS passengers(name VARCHAR(20) NOT NULL, address VARCHAR(20) NOT NULL,cityAddress VARCHAR(20), eMail VARCHAR(50) NOT NULL, TCKN VARCHAR(12) NOT NULL, PRIMARY KEY(TCKN) )");
	stmt->execute("CREATE TABLE IF NOT EXISTS airplanes(capacity INT NOT NULL, model VARCHAR(20) NOT NULL, atn VARCHAR(20) NOT NULL, PRIMARY KEY(atn) )");
	stmt->execute("CREATE TABLE IF NOT EXISTS flight( startingCity INT NOT NULL, destinationCity INT NOT NULL, date VARCHAR(20) NOT NULL, time VARCHAR(20) NOT NULL, atn VARCHAR(20) NOT NULL, PRIMARY KEY(date,time,atn), FOREIGN KEY(atn) REFERENCES airplanes(atn) ) ");
	stmt->execute("CREATE TABLE IF NOT EXISTS passenger (date VARCHAR(20) NOT NULL, atn VARCHAR(20) NOT NULL, startingCity INT NOT NULL, destinationCity INT NOT NULL, TCKN VARCHAR(12) NOT NULL, time VARCHAR(20) NOT NULL, KEY(date,atn,startingCity,destinationCity,TCKN, time),CONSTRAINT FOREIGN KEY(date,time,atn) REFERENCES flight(date,time,atn) ON DELETE CASCADE ON UPDATE CASCADE, FOREIGN KEY(TCKN) REFERENCES passengers(TCKN)  )");
}

// Returns nothing, used for infinite interface.
void mainLoop(sql::PreparedStatement *pstmt, sql::Statement *stmt, sql::Connection *con, sql::ResultSet *res) {
	while (1) {
		menu(pstmt, stmt, con, res);
		cout << endl << "Please press a button for returning to menu!";
		_getch();
		system("CLS");
	}
}

void menu(sql::PreparedStatement *pstmt, sql::Statement *stmt, sql::Connection *con, sql::ResultSet *res) {
	switch (welcomeMessage()) {
	case 1:
		newFlight(pstmt, stmt, con);
		break;
	case 2:
		sellTicket(pstmt, res, con);
		break;
	case 3:
		passengerQuery(pstmt, res, con);
		break;
	case 4:
		cancelFlight(pstmt, res, con, stmt);
		break;
	case 5:
		cancelTicket(pstmt, res, con);
		break;
	case 6:
		flightQuery(pstmt, res, con);
		break;
	case 7:
		listFlightOccupancyRate(pstmt, res, con);
		break;
	case 8:
		addAirplane(pstmt, con);
		break;
	}
}

// A Welcome message for user. Takes users menu choice from user.
int welcomeMessage() {
	int choice;

	menuInterface();
	cout << endl << "Sisteme Hos Geldiniz" << endl << endl << "Yapmak Istediginiz Islemi Seciniz:";
	cin >> choice;

	return choice;
}

void menuInterface() {
	cout << "1-New Flight" << endl;
	cout << "2-Sell Ticket" << endl;
	cout << "3-Passenger Info" << endl;
	cout << "4-Cancel a Flight" << endl;
	cout << "5-Cancel a Ticket" << endl;
	cout << "6-Flight Info" << endl;
	cout << "7-List by Flight Occupancy Rate" << endl;
	cout << "8-Add Airplane" << endl;
}

// Interface for addFlight function. Takes Airplane Tail Number(atn), date, starting city, destination city and time from user calls addFlight function.
void newFlight(sql::PreparedStatement *pstmt, sql::Statement *stmt, sql::Connection *con) {
	char atn[20], date[20], time[20];
	int startingCity, destinationCity;
	cout << "Ucagin Kuyruk numarasini giriniz:";
	cin >> atn;
	cout << "Sefer tarihini giriniz:";
	cin >> date;
	cout << "Ucagin kalkacagi sehrin plaka kodunu giriniz:";
	cin >> startingCity;
	cout << "Ucagin inecegi sehrin plaka kodunu giriniz:";
	cin >> destinationCity;
	cout << "Ucagin kalkacagi saati giriniz:";
	cin >> time;
	pstmt = con->prepareStatement("INSERT INTO flight(startingCity,destinationCity,date,atn,time) VALUES (?,?,?,?,?) ");
	//stmt->execute("SET FOREIGN_KEY_CHECKS = 1");
	addFlight(pstmt, atn, date, startingCity, destinationCity, time);
	//stmt->execute("SET FOREIGN_KEY_CHECKS = 0");
}

// Adds flight to DB, takes Airplane tail number(atn), date, starting city, destination city and time as input.
void addFlight(sql::PreparedStatement *pstmt, char *atn, char *date, int startingCity, int destinationCity, char* time) {
	pstmt->setInt(1, startingCity);
	pstmt->setInt(2, destinationCity);
	pstmt->setString(3, date);
	pstmt->setString(4, atn);
	pstmt->setString(5, time);
	pstmt->executeUpdate();
}

// Interface for selling tickets.
void sellTicket(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con) {
	int startingCity, destinationCity, ticketAmount;
	char date[20], time[20], atn[20], TCKN[12];
	cout << "Ucagin kalkacagi sehrin plaka kodunu giriniz:";
	cin >> startingCity;
	cout << "Ucagin inecegi sehrin plaka kodunu giriniz:";
	cin >> destinationCity;
	cout << "Sefer tarihini giriniz:";
	cin >> date;
	cout << "Bilet Sayisini Giriniz::";
	cin >> ticketAmount;

	listFlights(pstmt, res, con, startingCity, destinationCity, date, ticketAmount);

	cout << "Ucak saatini seciniz:";
	cin >> time;
	cout << "Ucak kanat numarasini giriniz:";
	cin >> atn;

	/*capacity = checkCapacity(pstmt, res, con, atn);

	pstmt = con->prepareStatement("SELECT * FROM passenger WHERE atn = ? AND date = ? AND time = ? ");
	pstmt->setString(1, atn);
	pstmt->setString(2, date);
	pstmt->setString(3, time);
	res = pstmt->executeQuery();

	howMuch = res->rowsCount();*/

	for (int i = 0; i < ticketAmount; i++) {
		cout << "Please enter TCKN:";
		cin >> TCKN;
		if (checkTCKN(pstmt, res, con, TCKN)) {
			addSoldTicket(pstmt, res, con, startingCity, destinationCity, TCKN, date, time, atn);
		}
		else {
			addPassenger(pstmt, res, con, TCKN);
		}
	}

}

// Lists Flights
void listFlights(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con, int startingCity, int destinationCity, char *date, int ticketAmount) {
	int count;
	pstmt = con->prepareStatement("SELECT * FROM flight WHERE flight.date = ? AND flight.startingCity = ? AND flight.destinationCity = ? ");
	pstmt->setString(1, date);
	pstmt->setInt(2, startingCity);
	pstmt->setInt(3, destinationCity);
	res = pstmt->executeQuery();

	/* Fetch in reverse = descending order! */
	count = res->rowsCount();
	for (int i = 0; i < count; i++) {
		res->next();
		cout << res->getString("date") << "\t" << res->getString("atn") << "\t" << res->getInt("startingCity") << "\t" << res->getInt("destinationCity") << "\t" << res->getString("time") << endl;
	}

	cout << "Aktarmali Ucuslar!" << endl;

	pstmt = con->prepareStatement("SELECT * FROM flight F1, flight F2 WHERE F1.startingCity = ? AND F1.destinationCity = F2.startingCity AND F2.destinationCity = ? ");
	//pstmt->setString(1, date);
	pstmt->setInt(1, startingCity);
	pstmt->setInt(2, destinationCity);
	res = pstmt->executeQuery();
	count = res->rowsCount();
	for (int i = 0; i < count; i++) {
		res->next();
		cout << res->getString("date") << "\t" << res->getString("atn") << "\t" << res->getInt("startingCity") << "\t" << res->getInt("destinationCity") << "\t" << res->getString("time") << endl;

	}
}

int checkCapacity(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con, char *atn) {
	pstmt = con->prepareStatement("SELECT * FROM airplanes WHERE atn = 'TC-YGZ' ");
	//pstmt->setString(1, atn);
	res = pstmt->executeQuery();

	return res->getInt("capacity");
}

// Checks TCKN
bool checkTCKN(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con, char* TCKN) {
	int count;
	pstmt = con->prepareStatement("SELECT * FROM passengers WHERE TCKN = ?");
	pstmt->setString(1, TCKN);
	res = pstmt->executeQuery();
	count = res->rowsCount();
	if (count == 0)
		return false;
	else if (count == 1)
		return true;
	else
		cout << "Warning multiple record for same TCKN!";
}

// Adds ticket to passenger table.
void addSoldTicket(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con, int startingCity, int destinationCity, char* TCKN, char *date, char *time, char *atn) {
	pstmt = con->prepareStatement("INSERT INTO passenger() VALUES (?,?,?,?,?,?)");
	pstmt->setString(1, date);
	pstmt->setString(2, atn);
	pstmt->setInt(3, startingCity);
	pstmt->setInt(4, destinationCity);
	pstmt->setString(5, TCKN);
	pstmt->setString(6, time);
	pstmt->executeUpdate();
}

// Adds passemger to passenger table.
void addPassenger(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con, char* TCKN) {
	char name[20], address[20], cityAddress[20], eMail[50];
	cout << "Enter passsengers name:";
	cin >> name;
	cout << "Enter passengers address:";
	cin >> address;
	cout << "Enter passengers city address:";
	cin >> cityAddress;
	cout << "Enter passengers eMail:";
	cin >> eMail;

	pstmt = con->prepareStatement("INSERT INTO passengers() VALUES (?,?,?,?,?)");
	pstmt->setString(1, name);
	pstmt->setString(2, address);
	pstmt->setString(3, cityAddress);
	pstmt->setString(4, eMail);
	pstmt->setString(5, TCKN);
	pstmt->executeUpdate();

	sellTicket(pstmt, res, con);
}

// Makes query about passenger.
void passengerQuery(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con) {
	system("CLS");
	int count;
	char TCKN[12];
	cout << "TCKN giriniz:";
	cin >> TCKN;
	pstmt = con->prepareStatement("SELECT * FROM passenger WHERE TCKN = ?");
	pstmt->setString(1, TCKN);
	res = pstmt->executeQuery();
	count = res->rowsCount();
	if (count == 0) {
		cout << endl << "Boyle bir yolcu bulunmamaktadir!";
	}
	else {
		for (int i = 0; i < count; i++) {
			res->next();
			cout << res->getString("date") << "\t" << res->getString("atn") << "\t" << res->getInt("startingCity") << "\t" << res->getInt("destinationCity") << "\t" << res->getString("TCKN") << res->getString("time") << endl;
		}
	}
}

// Cancels a flight. Takes Airplane Tail Number, date and time from user and deletes that flight from flights table and deletes tickets of that flight from passenger table.
void cancelFlight(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con, sql::Statement *stmt) {
	char date[20], atn[20], time[20];

	cout << "Enter Airplane Tail Number:";
	cin >> atn;
	cout << "Enter flight date:";
	cin >> date;
	cout << "Enter time:";
	cin >> time;
	//stmt->execute("SET FOREIGN_KEY_CHECKS = 1");
	pstmt = con->prepareStatement("DELETE FROM flight WHERE time = ? AND date = ? and atn = ?");
	pstmt->setString(1, time);
	pstmt->setString(2, date);
	pstmt->setString(3, atn);
	res = pstmt->executeQuery();
	//stmt->execute("SET FOREIGN_KEY_CHECKS = 0");
}

// Cancels a ticket. Takes TCKN, date and Airplane Tail Number as input from user and deletes that ticket from passenger table.
void cancelTicket(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con) {
	char TCKN[12];
	char date[20], atn[20];

	cout << "Enter TCKN:";
	cin >> TCKN;
	cout << "Enter flight date:";
	cin >> date;
	cout << "Enter Airplane Tail Number:";
	cin >> atn;

	pstmt = con->prepareStatement("DELETE FROM passenger WHERE TCKN = ? AND date = ? and atn = ?");
	pstmt->setString(1, TCKN);
	pstmt->setString(2, date);
	pstmt->setString(3, atn);
	res = pstmt->executeQuery();
}

// Makes query about a flight.
void flightQuery(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con) {
	system("CLS");
	char date[20], atn[20], time[20];
	int count;
	cout << "Enter Flight Date:";
	cin >> date;
	cout << "Enter Flight Time:";
	cin >> time;
	cout << "Enter Airplane Tail Number:";
	cin >> atn;
	pstmt = con->prepareStatement("SELECT passengers.name,passengers.address,passengers.cityAddress,passengers.eMail,passengers.TCKN FROM flight, passengers, passenger WHERE passengers.TCKN = passenger.TCKN AND passenger.date = ? AND flight.atn = ? AND flight.date = ? AND flight.time = ?");
	pstmt->setString(1, date);
	pstmt->setString(2, atn);
	pstmt->setString(3, date);
	pstmt->setString(4, time);
	res = pstmt->executeQuery();
	count = res->rowsCount();
	for (int i = 0; i < count; i++) {
		res->next();
		cout << res->getString("name") << "\t" << res->getString("address") << "\t" << res->getString("cityAddress") << "\t" << res->getString("eMail") << "\t" << res->getString("TCKN") << endl;
	}
}

void listFlightOccupancyRate(sql::PreparedStatement *pstmt, sql::ResultSet *res, sql::Connection *con) {
	int count;
	pstmt = con->prepareStatement("SELECT * FROM passenger WHERE ");
	res = pstmt->executeQuery();

}

// Interface for addAirplanetoDB. Takes capacity, model and Airplane Tail Number(atn) as input from user.
void addAirplane(sql::PreparedStatement *pstmt, sql::Connection *con) {
	system("CLS");
	int capacity;
	char model[20], atn[20];
	cout << "Enter the plane capacity:";
	cin >> capacity;
	cout << "Enter the model of plane:";
	cin >> model;
	cout << "Enter Airplane Tail Number:";
	cin >> atn;
	pstmt = con->prepareStatement("INSERT INTO airplanes(capacity,model,atn) VALUES (?,?,?)");
	addAirplanetoDB(pstmt, capacity, model, atn);
}

// Adds airplane to DB, takes capacity, model, Airplane tail number(atn) as input.
void addAirplanetoDB(sql::PreparedStatement *pstmt, int capacity, char* model, char* atn) {
	pstmt->setInt(1, capacity);
	pstmt->setString(2, model);
	pstmt->setString(3, atn);
	pstmt->executeUpdate();
}

// Used in catch prints error to console.
void errorMessages(sql::SQLException &e/*, sql::PreparedStatement *pstmt, sql::Statement *stmt, sql::Connection *con, sql::ResultSet *res */) {
	cout << "# ERR: SQLException in " << __FILE__;
	cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
	cout << "# ERR: " << e.what();
	cout << " (MySQL error code: " << e.getErrorCode();
	cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	/*cout << "Wrong data entry from user!"
	mainLoop(pstmt,stmt,con,res);*/
	_getch();
}