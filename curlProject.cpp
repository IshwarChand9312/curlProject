// curlProject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define CURL_STATICLIB
#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include "curl/curl.h"
#include "json/json.h"
#include "sqlite3.h"

using namespace std;

static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

// callback process each SELECTE statement executed within SQL argument
static int callback(void* data, int argc, char** argv, char** azColName) {
	int i;

	fprintf(stderr, "%s: ", (const char*)data);

	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	printf("\n");
	return 0;
}


int main()
{

    string result;
	/* Fetching data from website using CURL*/
    CURL* curl;
	CURLcode res;
	fstream  out("output.txt");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://dummy.restapiexample.com/api/v1/employees");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (CURLE_OK != res) {
            std::cerr << "CURL error: " << res << '\n';
        }
    }
    curl_global_cleanup();
    std :: cout << result << "\n\n";
	

	// Getting Data from output file 
    std::string temp = "";
    std::string str = "";
    while (getline(out, temp)) {

        str += temp;
    }
    
    Json::Value root;
    Json::Reader  reader;

	/* Parsing  data from string to JSON*/
    if (!reader.parse(str, root)) {
       cout << reader.getFormattedErrorMessages() << "\n";
	   return 0;
    }
	// Fetching data from JSON array
    Json::Value data = root["data"];
	
	// Creating SQL database
	sqlite3* db;
	int rc;
	rc = sqlite3_open("test.db", &db);
	if (rc) {
		cout << "Can't Open Database ";
	}
	else {
		cout << "Opened Database Succesfully";
	}




	// Creating SQL table 
	const char* sql;
	char* zErrmsg = 0;

	sql = "CREATE TABLE EMPLOYEE("
		"ID INT PRIMARY KEY NOT NULL,"
		"EMPLOYEE_NAME TEXT NOT  NULL,"
		"SALARY REAL ,"
		"EMPLOYEE_AGE INT NOT NULL,"
		"PROFILE_IMAGE CHAR(100) );"
		;

	rc = sqlite3_exec(db, sql, NULL, 0, &zErrmsg);
	if (rc != SQLITE_OK) {
		cout << "sqlite Error : " << zErrmsg;
		//sqlite3_free(zErrmsg);
	}
	else {
		cout << "Table created Succesfully ";
	}

	char* messageError;
	

	// Inserting Json Data to SQL table 
	for (int i = 0; i < data.size(); i++) {
		
		Json::Value employee = data[i];
		string str = employee["id"].asString() + ",'" + employee["employee_name"].asString() + "'," +
			employee["employee_salary"].asString() + "," + employee["employee_age"].asString() + ",'" + employee["profile_image"].asString() + "'";

		string sql =  "INSERT INTO EMPLOYEE(ID, EMPLOYEE_NAME, SALARY, EMPLOYEE_AGE, PROFILE_IMAGE) VALUES(" + str + ");";

		int exit = sqlite3_exec(db, sql.c_str(), NULL, 0, &messageError);
	}


	// Printing SQL table data 
	string sql_select = "SELECT * FROM EMPLOYEE";
	const char* dat = "Callback function called";
	int rt = sqlite3_exec(db, sql_select.c_str(), callback, (void*)dat, &messageError);


	out.close();
	sqlite3_close(db);

	return 0;

}
