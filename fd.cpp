/**
 * fdupes - программа для поиска и удаления одинаковых файлов
 *
 * g++ fd.cpp /usr/lib64/libboost_system.so.1.66.0 /usr/lib64/libboost_filesystem.so.1.66.0
 *
 * Copyright (c) 2020, Dasha Efimova<dashulya0301178@gmail.com>
 *
 * This code is licensed under a MIT-style license.
 */

#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <map>

using namespace std;

namespace fs = boost::filesystem;

bool comparison(fs::directory_entry p1, fs::directory_entry p2); //Функция для побайтового сравнения файлов

unsigned long hashCalculation(fs::directory_entry p); //Функция для подсчёта хэша файла

int main(int argc, char *argv[]) {

    /*Проверяем, введен ли путь*/
    if (argc < 2) {
	cout << "Choose a directory!\n";
	exit(EXIT_FAILURE);
    }

    /*Запоминаем указанный путь к файлу*/
    fs::path checkPath(argv[1],argv[1] + strlen(argv[1]));

    /*Проверяем, существует ли указанный путь*/
    if (!fs::exists(checkPath)) {
	cout << "Incorrect path!\n";
	exit(EXIT_FAILURE);
    }

    /*Проверяем, является ли директорией*/
    if (!fs::is_directory(checkPath)) {
	cout << "Incorrect directory!\n";
	exit(EXIT_FAILURE);
    }

    /*Создаем контейнер для хранения файлов для сравнения их по размеру*/
    map<unsigned long, vector<fs::directory_entry> > fileSizeMap;

    /*Рекурсивно обходим каталог*/
    for(auto& p:fs::recursive_directory_iterator(argv[1])) {
	/*Если путь файла соответствует обучному файлу, то*/
	if(fs::is_regular_file(p)) {
	    /*В контейнер добавляем путь файла по его размеру*/
	    fileSizeMap[file_size(p.path())].push_back(p);
	}
    }

    /*Создаем контейнер для хранения файлов для сравнения их по хэшам*/
    map<unsigned long, vector<fs::directory_entry> > fileHashMap;

    /*От начала до конца обходим контейнер файлов для сравнения по размеру*/
    for(auto iterator1 = fileSizeMap.begin(); iterator1 != fileSizeMap.end(); iterator1++) {
	/*Если размер файла больше 1, то*/
	if (iterator1->second.size() > 1) {
	    /*От начала до конца файла*/
	    for (auto iterator2 = iterator1->second.begin(); iterator2 != iterator1->second.end(); iterator2++) {
		/*Добавляем файл в контейнер на основе рассчитанного хэша*/
		fileHashMap[hashCalculation(*iterator2)].push_back(*iterator2);
	    }
	}
    }

    fileSizeMap.clear(); //Очищаем контейнер для сравнения файлов по размерам

    /*Создаем контейнер для хранения файлов для сравнения их побайтово*/
    map<unsigned long, vector<fs::directory_entry> > fileByteMap;

    /*От начала до конца обходим контейнер файлов для сравнения по хэшам*/
    for(auto iterator1 = fileHashMap.begin(); iterator1 != fileHashMap.end(); iterator1++) {
	/*Если размер файла больше 1, то*/
	if(iterator1->second.size() > 1) {
	    bool flag = false;
	    /*Файл из контейнера для сравнения хэшей попадает в контейнер для сравнения побайтово*/
	    fileByteMap[fileByteMap.size()].push_back(iterator1->second[0]);
	    /*Пока не дойдем до конца размера значения контейнера файлов по хэшу*/
	    for (unsigned long i = 1; i < iterator1->second.size(); i++) {
		/*Пока не дойдем до конца размера контейнера для сравнения побайтово*/
		for (unsigned long j = 0; j < fileByteMap.size(); j++) {
		    /*Побайтовое сравнение файлов*/
		    if (comparison(fileByteMap[j].front(), iterator1->second[i])) {
			/*Распределяем одинаковые файлы*/
			fileByteMap[j].push_back(iterator1->second[i]);
			flag = true;
			break;
		    }
		}
		if(flag == false) {
		    /*Добавляем в контейнер для побайтового сравнения файлов файл*/
		    fileByteMap[fileByteMap.size()].push_back(iterator1->second[i]);
		}
	    }
	}
    }

    fileHashMap.clear(); //Очищаем контейнер для сравнения файлов по хэшам

    /*От начала до конца обходим контейнер для сравнения файлов побайтово и выводим одинаковые файлы пользователю*/
    for (auto iterator1 = fileByteMap.begin(); iterator1 != fileByteMap.end(); iterator1++) {
	/*Если размер файла больше 1, то*/
	if (iterator1->second.size() > 1) {
	    unsigned long fileNumber = 0;
	    cout << "\nDetected same files:\n";
	    /*От начала до конца файла*/
	    for(auto iterator2 = (iterator1->second).begin(); iterator2 != (iterator1->second).end(); iterator2++) {
		fileNumber++;
		/*Выводим путь к нужным файлам*/
		cout << fileNumber << ". " << iterator2->path() << "\n";
	    }
	    /*Пользователь выбирает, какой файл удалить*/
	    cout << "\nWhat would you like to delete? Press '0' to exit the program.\n";
	    cin >> fileNumber;
	    //if (fileNumber == 0)
		//break;
	   // fs::remove(iterator1->second[fileNumber-1]); //Удаляем файл под выбранным номером
		
		for (int i=0; i< iterator1->second.size(); i++){
		if (i != fileNumber-1)   
	    fs::remove(iterator1->second[i]);  // удаление файла по выбранному номеру
	}
    }
}

/*Функция для побайтового сравнения файлов*/
bool comparison(fs::directory_entry p1, fs::directory_entry p2){

    /*Открываем поток для чтения файла в двоичном коде, преобразуя при этом путь к файлу в символьную строку*/
    ifstream f1(p1.path().c_str(), ios::binary);
    ifstream f2(p2.path().c_str(), ios::binary);
    bool comparisonResult = true;

    char s1, s2;
    /*Пока получаем символы из первого и второго файлов*/
    while (f1.get(s1) && f2.get(s2)) {
	/*Если символы различны, то*/
	if (s1 != s2) {
	    /*Файлы различны*/
	    comparisonResult = false;
	    break;
	}
    }
    return comparisonResult;
}

/*Функция для подсчёта хэша файла*/
unsigned long hashCalculation(fs::directory_entry p) {
    unsigned long fileHash=0;
    /*Открываем поток для чтения файла, преобразуя при этом путь к файлу в символьную строку*/
    ifstream f(p.path().c_str(), ios::out);

    char s;
    /*Пока получаем символы из файла*/
    while (f.get(s)) {
	/*подсчитываем хэш файла*/
	fileHash += s;
    }
    return fileHash;
}
