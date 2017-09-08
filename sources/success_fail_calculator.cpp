#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include "dirent.h"
#include <windows.h>
#include<ctime>
#include <stdlib.h> //strtol()
using namespace std;

string error_code;
ofstream log_file;

string GetErrorTime()
{
	time_t t = time(0);
	struct tm* now = localtime(&t);
	stringstream ss;
	ss  << (now->tm_year + 1900) << '-' 
         << (now->tm_mon + 1) << '-'
         << now->tm_mday << ' '
         << now->tm_hour <<':'
         << now->tm_min << ':'
         << now->tm_sec;
	return ss.str();
}

string GetTime(string file)
{
	ifstream in_file;
	in_file.open(file.c_str());
	if(!in_file)
	{
		log_file << GetErrorTime() << ",Could not open file:" << file << endl;
		return "Could not get EXPORT_DATE"; 
	}
	string row;
	while(getline(in_file, row))
	{
		if(row.find("EXPORT_DATE = ")!=string::npos)
		{
			char *endptr;
			if(strtol((row.substr(row.find("EXPORT_DATE = ")+14, 8)).c_str(), &endptr, 10))
				return (row.substr(row.find("EXPORT_DATE = ")+14,8));
			else
			{
				log_file << GetErrorTime() << ",Could not get EXPORT_DATE:" << file << endl; 
			
				return "Could not get EXPORT_DATE";
			}
		}
			
	}
	return "Could not get EXPORT_DATE"; 
}


void read_dir(string dir_path, ofstream& out_file, time_t now_t)
{
	DIR * dir;
	struct dirent *ent;
	string dir_name;
	if((dir = opendir(dir_path.c_str()))!=NULL)
	{
		while((ent = readdir (dir)) != NULL)
		{
			dir_name = ent->d_name;
			if(dir_name.find('.')==std::string::npos)
				read_dir(dir_path+"\\"+dir_name, out_file, now_t);
			else if(dir_name.find("REWORK")==std::string::npos &&(dir_name.find("FUJ")!=std::string::npos || dir_name.find("oRC")!=std::string::npos))//Exclude rework
			{
				string record_date = GetTime(dir_path+"\\"+dir_name);
				if(record_date == "Could not get EXPORT_DATE" || record_date == "Could not get EXPORT_DATE")
					continue;
				
				struct tm record;
				record.tm_year = atoi(record_date.substr(0,4).c_str()) - 1900;
				record.tm_mon = atoi(record_date.substr(4,2).c_str()) - 1;
				record.tm_mday = atoi(record_date.substr(6,2).c_str());
				record.tm_hour = 0;
				record.tm_min = 0;
				record.tm_sec = 0;
				
				time_t record_t = mktime(&record);
				if(difftime(now_t, record_t) == 86400)
				{
					stringstream ss;	
					ss << record.tm_year + 1900;
					ss.fill('0');ss.width(2); ss << record.tm_mon + 1;
					ss.fill('0');ss.width(2); ss << record.tm_mday;
					out_file << error_code << ','<< dir_name <<','<< dir_path << endl;
					ss.str("");
					ss.clear();
				}
			}
			else
			{}
		}
		
	}
	else
		log_file << GetErrorTime() << ',' << "Open "<< dir_path << " fail." << endl;
	closedir (dir);
	return;
}

void Output(string path, string file_name, string date, double fail_lots, double success_lots, int mode)
{
	ifstream in_file;
	in_file.open((path + file_name).c_str());
		
	if(in_file)
	{
		ofstream out_file;
		out_file.open((path + "temp_" + file_name).c_str());
		
		string row;
		bool inserted = false;
		while(getline(in_file, row))
		{
			string record_date = row.substr(0, row.find(","));
			if((record_date[record_date.length() - 1] == '0'&& record_date[record_date.length() - 2] == '0') || atoi(record_date.c_str()) == atoi(date.c_str()))//reomve wrong date
				continue;
			if(mode == 3 && record_date == date)
				continue;
			if(mode != 3 && !inserted && atoi(record_date.c_str()) > atoi(date.c_str()))
			{
				if((success_lots+fail_lots)!=0)
					out_file << date << ',' << "success_lots:" << success_lots << ",fail_lots:" << fail_lots << ",success_rate:" << success_lots/(success_lots+fail_lots) << endl;
				else
					out_file << date << ',' << "Can not find any lots today." << endl;
				inserted = true;
			}
			out_file << row << endl;
		}
		if(mode != 3 && !inserted)
		{
			if((success_lots+fail_lots)!=0)
				out_file << date << ',' << "success_lots:" << success_lots << ",fail_lots:" << fail_lots << ",success_rate:" << success_lots/(success_lots+fail_lots) << endl;
			else
				out_file << date << ',' << "Can not find any lots today." << endl;
		}
		out_file.close();
	}
	else
		return;
	
	in_file.close();
	remove((path + file_name).c_str());
	rename((path + "temp_" + file_name).c_str(), (path + file_name).c_str());
}


int main(){

	while(1)
	{
	time_t now_t = time(0);
	struct tm* now = localtime(&now_t);	
	
	int mode = 0;
	string remove_date = "";
	cout << "Select mode: 1(Auto)/2(Insert)/3(Remove)" << endl << "mode:";
	cin >> mode;
	if(mode!=1&&mode!=2&&mode!=3)
	{
		cout << "input wrong number " << endl;
		return 1;
	}
	if(mode == 3)
	{
		int remove_year;
		int remove_mon;
		int remove_mday;
		cout << "Input the removing date." << endl << "year:";
		cin >> remove_year;
		cout << "month:";
		cin >> remove_mon;
		cout << "date:";
		cin >> remove_mday;
		now->tm_year = remove_year - 1900;
		now->tm_mon = remove_mon - 1;
		now->tm_mday = remove_mday + 1;
	}
	
	if(mode == 2){
		int insert_year;
		int insert_mon;
		int insert_mday;
		cout << "Input the inserting date." << endl << "year:";
		cin >> insert_year;
		cout << "month:";
		cin >> insert_mon;
		cout << "date:";
		cin >> insert_mday;
		now->tm_year = insert_year - 1900;
		now->tm_mon = insert_mon - 1;
		now->tm_mday = insert_mday + 1;
	}
	now->tm_hour = 0;
	now->tm_min = 0;
	now->tm_sec = 0;
	now_t = mktime(now);
	
	time_t last_t = now_t - 86400;
	struct tm* last = localtime(&last_t);
	
	
	stringstream ss;
	ss << last->tm_year + 1900;
	ss.fill('0');ss.width(2); ss << last->tm_mon + 1;
	ss.fill('0');ss.width(2); ss << last->tm_mday;

	string statistic_dir = "\\\\10.1.11.73\\di_mac_mes\\Success_rate_statistc";
	string camPC_ng_dir = "\\\\10.1.11.73\\di_mac_mes\\ng";
	string camPC_finish_dir = "\\\\10.1.11.73\\di_mac_mes\\finish";
	string register_ng_dir = "\\\\10.1.11.73\\di\\FUJI\\camdata\\register_error";
	string register_ok_dir = "\\\\10.1.11.73\\di\\FUJI\\camdata\\register_ok";
	
	
	CreateDirectory((statistic_dir+"\\CAM_PC").c_str(), NULL);	
	log_file.open((statistic_dir+"\\CAM_PC\\log.csv").c_str(), std::ofstream::out | std::ofstream::app);//Record if fail to open directory or get file create time
	
	/*************************************************************************************/
	/*CAM PC TRANSFER ERROR*/
	/*************************************************************************************/
	ofstream out_file;//Out put ng code and file name in .csv-form file named by date	
	string out_file_name = ss.str() + ".csv";

if(mode != 3)
{
	out_file.open((statistic_dir+"\\CAM_PC\\"+out_file_name).c_str());
	out_file << "error_code,Lot,path"<< endl;//Header row of csv
	//scan ng directory
	DIR *dir;
	struct dirent *ent;
	string dir_name;
	if ((dir = opendir (camPC_ng_dir.c_str())) != NULL) 
	{
 	 	/* print all the files and directories within directory */
 	 	while ((ent = readdir (dir)) != NULL) 
		{
	  	  dir_name = ent->d_name; 
	
			if(dir_name.find('.')==std::string::npos)
			{
				error_code = dir_name;
				read_dir(camPC_ng_dir+"\\"+dir_name, out_file, now_t);
			}
		}
		closedir (dir);
	} 
	else
	{
	  /* could not open directory */
		log_file << GetErrorTime() << ',' << "Could not open " << camPC_ng_dir << endl; 
 		perror ("");
		return 1;
	}
	cout << ss.str() << "-scan error directory done" << endl;
	
	//scan finish directory
	string file_name; 
	if ((dir = opendir (camPC_finish_dir.c_str())) != NULL) 
	{
 		 while ((ent = readdir (dir)) != NULL) 
		{
	    	file_name = ent->d_name; 
			if(file_name.find("REWORK")==std::string::npos &&(file_name.find("FUJ")!=std::string::npos || file_name.find("oRC")!=std::string::npos))
			{
				string file_date = GetTime(camPC_finish_dir+"\\" + file_name);
				if(file_date == ss.str())
				{
					error_code = "Success";
					out_file << error_code << ',' << file_name << ',' <<  camPC_finish_dir << endl;
				}
			}
		}
		closedir (dir);	
	} 
	else
	{			
	  /* could not open directory */
		log_file << GetErrorTime() << ',' << "Could not open " << camPC_finish_dir << endl;
 		perror ("");
		return 1;
	}
	out_file.close();	
	cout << ss.str() << "-scan finish directory done" << endl;
	
	//Calculate success rate
	ifstream in_file;
	in_file.open((statistic_dir+"\\CAM_PC\\"+out_file_name).c_str());
	string row;
	float fail_lots = 0.0, success_lots = 0.0;
	getline(in_file, row);//Remove the header row
	while(getline(in_file, row))
	{ 
		if(row.find("Success") == std::string::npos)
			fail_lots++;
		else
			success_lots++;
	}
	in_file.close();
	//cout << fail_lots << ',' << success_lots << endl;	

	Output(statistic_dir+"\\", "Success_rate_statistic(CAM_PC).csv", ss.str(), fail_lots, success_lots, mode);	
	log_file.close();
	cout  << ss.str() << "-CAM PC TRANSFER ERROR done" << endl;
}
else
{
	remove((statistic_dir+"\\CAM_PC\\"+out_file_name).c_str());
	Output(statistic_dir+"\\", "Success_rate_statistic(CAM_PC).csv", ss.str(), 0, 0, mode);
	cout << ss.str() << "-CAM PC log removed." << endl;
}
	/*************************************************************************************/
	/*FUJI DI TRANSFER ERROR*/
	/*************************************************************************************/
	string mechine_number;
	for(int i = 1; i <= 8; i++)
	{
		if(i==1){mechine_number = "EDi_FUJ-01";}
		else if(i==2){mechine_number = "EDi_FUJ-02";}
		else if(i==3){mechine_number = "EDi_FUJ-03";}
		else if(i==4){mechine_number = "EDi_FUJ-04";}
		else if(i==5){mechine_number = "EDi_AD365-01";}
		else if(i==6){mechine_number = "EDi_AD500-01";}
		else if(i==7){mechine_number = "EDi_AD500-02";}
		else {mechine_number = "EDi_AD500-03";}
		CreateDirectory((statistic_dir+"\\"+mechine_number).c_str(), NULL);//Create directory named by machine number if it doesn't exist
	if(mode != 3)
	{
		log_file.open((statistic_dir+"\\"+mechine_number+"\\"+"log.csv").c_str(), std::ofstream::out | std::ofstream::app);
		out_file.open((statistic_dir+"\\"+mechine_number+"\\"+ss.str()+".csv").c_str());
		out_file << "Error Code,Recipe name" << endl;
		DIR *dir;
		struct dirent *ent;
		string dir_name;
		////scan register_error directory
		if ((dir = opendir ((register_ng_dir+"\\" + mechine_number).c_str())) != NULL) 
		{
 	 		while ((ent = readdir (dir)) != NULL) 
			{
	    		string file_name = ent->d_name;
				if(file_name!=("_error.log")&&file_name.find("error")!=std::string::npos)
	    		{
	    			string file_date = file_name.substr(0, 8);
	    			if(file_date == ss.str())
	    			{
						ifstream in_file;
	    				in_file.open((register_ng_dir+"\\"+mechine_number+"\\"+file_name).c_str());
	    				string row;
	    				getline(in_file, row);
	    				out_file << row << ',' << file_name << endl;
	    				in_file.close();
	    			} 
				}	    		
			}
			closedir(dir);
		}
		else
		{
			log_file << GetErrorTime() << ",Could not open " << register_ng_dir << "\\" << mechine_number << endl;
			perror ("");
			return 1;
		}
		cout  << ss.str() << "-scan register_error directory done" << endl;
		
		//scan register_ok directory
		if ((dir = opendir ((register_ok_dir+"\\"+mechine_number).c_str())) != NULL) 
		{
 	 		while ((ent = readdir (dir)) != NULL) 
			{
	    		string file_name = ent->d_name;
	    		string file_date = file_name.substr(0, 8);
	    		if(file_date == ss.str())
					out_file << "Success," << file_name << endl;
	    		
			}
			closedir(dir);
		}
			else
		{
			log_file << GetErrorTime() << ",Could not open " << register_ok_dir << "\\" << mechine_number << endl;
			perror ("");
			return 1;
		}
		out_file.close();
		cout  << ss.str() << "-scan register_ok directory done" << endl;
		
		//Calculate success rate
		ifstream in_file;
		in_file.open((statistic_dir+"\\"+mechine_number+"\\"+ss.str()+".csv").c_str());
		double fail_lots = 0.0, success_lots = 0.0;
		string row;
		getline(in_file, row);//Remove the header row
		while(getline(in_file, row))
		{
			if(row.find("Success")!=std::string::npos)
				success_lots++;
			else
				fail_lots++;
		}
		in_file.close();
		
		Output(statistic_dir+"\\", "Success_rate_statistic("+mechine_number+").csv", ss.str(), fail_lots, success_lots, mode);
		log_file.close();

		cout  << ss.str() << "-"<< mechine_number << "FUJI DI TRANSFER ERROR" << endl;
	}
	else
	{
		remove((statistic_dir+"\\"+mechine_number+"\\"+ss.str()+".csv").c_str());
		Output(statistic_dir+"\\", "Success_rate_statistic("+mechine_number+").csv", ss.str(), 0, 0, mode);
		cout << ss.str() << "-"<< mechine_number << " log removed" << endl;
	}
	}
	//Clean stringstream 
	ss.str(std::string());
	ss.clear();
	if(mode == 1)
	{
		cout << ss.str() << " scanning finished." << endl;
		_sleep(86400000);//Run once each 24 hours
	}
	}
	
	
	return 0;
}
