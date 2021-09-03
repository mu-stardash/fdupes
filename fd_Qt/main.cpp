/**
 * fdupes - программа для поиска и удаления одинаковых файлов
 *
 * Copyright (c) 2020, Dasha Efimova<dashulya0301178@gmail.com>
 *
 * This code is licensed under a MIT-style license.
 */
 
#include <iostream>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QString>
#include <QMap>
#include <QSet>
#include <stdlib.h>

using namespace std;

void filesDeletion(QMap<int, QVector<QString>>, int); //Функция для удаления файлов

bool comparison(QString p1, QString p2); //Функция для побайтового сравнения файлов

unsigned long hashCalculation(QString path); //Функция для подсчёта хэша файла

int main(int argc, char *argv[])
{
	/*Создаем контейнер для хранения файлов для сравнения их по размеру*/
    QMap<qint64, QSet<QString>> fileSizeMap;
	
	/*Создаем контейнер для хранения файлов для сравнения их по хэшам*/
     QMap<unsigned long, QVector<QString>> fileHashMap;
	 
	/*Создаем контейнер для хранения файлов для сравнения их побайтово*/
    QMap<int, QVector<QString>> fileByteMap;

    /*Проверяем, введен ли путь*/
    if (argc < 2) {
		cout << "Choose a directory" << endl;
        return 0;
    }

    /*Запоминаем указанный путь к файлу*/
    QDir path(argv[1]);

    /*Проверяем, существует ли указанный путь*/
    if (!path.exists()) {
       std::cout <<  "Incorrect path" << endl;
        return 1;
    }

    /*Выполняем обход дерева каталогов*/
    QDirIterator iterator(path, QDirIterator::Subdirectories);

		/*Пока не пройдем все файлы*/
		while (iterator.hasNext()) {
			QFile f(iterator.next()); //Передаем имя файла
			QFileInfo info(f); //Узнаем информацию о файле

        /*Распределяем файлы по размеру*/
        fileSizeMap[info.size()].insert(f.fileName());
    }

    QMap<qint64, QSet<QString>>::iterator s_iterator;

    /*Выполняем поиск файлов с одинаковым хэшем*/
    for (s_iterator = fileSizeMap.begin(); s_iterator != fileSizeMap.end(); ++s_iterator) {
        QSet<QString> paths = s_iterator.value(); //В paths будет путь файла
		/*Если существует путь к файлу*/
        if (paths.size() > 1) {
            QSet<QString>::iterator p_iterator;
			/*От начала до конца пути*/
            for (p_iterator = paths.begin(); p_iterator != paths.end(); ++p_iterator) {
				/*Получаем информацию о данном участке пути*/
                QFileInfo info(*p_iterator);
				/*Если это директория, то продолжаем*/
                if (!info.isFile())
                    continue;
				/*Рассчитываем хэш*/
                unsigned long fileHash = hashCalculation(*p_iterator);
				/*Добавляем файл в контейнер на основе расчитанного хэша*/
                fileHashMap[fileHash].push_back(*p_iterator);
            }
        }
    }

    QMap<unsigned long, QVector<QString>>::iterator h_iterator;
    int index = 0;

    /*От начала до конца обходим контейнер файлов для сравнения по хэшу для отбора одинаковых*/
    for (h_iterator = fileHashMap.begin(); h_iterator != fileHashMap.end(); ++h_iterator) {
        QVector<QString> paths = h_iterator.value();
		/*Попарно сравниваем файлы*/
        for (int i = 0; i < paths.size(); i++) {
            for (int j = 0; j < paths.size(); j++) {
				/*Если побайтово фацлы равны,то*/
                if (comparison(paths[i], paths[j]) && paths[i] != paths[j] && !fileByteMap[index].contains(paths[j])) {
                    /*Добавляем файл из контейнера для хэша в  контейнер для побайтового сравнения*/
                    fileByteMap[index].push_back(paths[j]);
                }
            }
        }
        index++;
    }

    /*Если одинаковых файлов не нашлось*/
    if (fileByteMap.size() < 1) { 
        cout << "Similar files not found!" << endl;
        return 0;
    }
    /*Если одинаковые файлы нашлись*/
    cout << "Similar files found!" << endl;

    QMap<int, QVector<QString>>::iterator b_iterator;
	

    /*От начала до конца обходим контейнер для сравнения файлов побайтово и выводим одинаковые файлы пользователю*/
    for (b_iterator = fileByteMap.begin(); b_iterator != fileByteMap.end(); ++b_iterator) {
        QVector<QString> paths = b_iterator.value();

		cout << " Найдено " << b_iterator.key() + 1 << " одинаковых файла" << endl;

        /*Выводим пути к найденным файлам*/
        for(int i = 0; i < paths.size(); i++) {
        qInfo() << paths[i];
        }

        cout << "-------------------" << endl;
    }

    int input = 0;

    /*Пользователь выбирает, какой файл удалить*/
    cout << "Choose a group of files" << endl;
    cin >> input;
    filesDeletion(fileByteMap, input-1);
	return 0;
}

/*Функция удаления файлов*/
void filesDeletion(QMap<int, QVector<QString>> files_block, int block_num) {

    /*Создаем массив файлов*/
    QVector<QString> files = files_block[block_num];

    cout << "-------------------" << endl;

    /*Выводим файлы*/
    for (int i = 1; i <= files.size(); i++) {
        qInfo() << i << files[i-1];
    }

    cout << "-------------------" << endl;

    /*Пользователь выбирает, какой файл удалить*/
    int fileNumber = 0;
    cout << "Which file would you like to delete?" << endl;
    cin >> fileNumber;

    for (int i = 1; i <= files.size(); i++) {
	    /*Если совпало с введенным номером*/
        if (i == fileNumber) {
			/*То удаляем файл*/
            QFile::remove(files[i-1]);
        }
    }
    cout << "File deleted successfully!"<< endl;
}

/*Функция для побайтового сравнения файлов*/
bool comparison(QString p1, QString p2) {
    QFile file1(p1); //передаем первый файл
    QFile file2(p2); //передаем второй файл

    bool comparisonResult = true;
    char s1, s2;

    /*Пока не кончился первый или второй файл*/
    while (!file1.atEnd() || !file2.atEnd()) {
		/*Получаем их символы*/
        file1.read(&s1, 1);
        file2.read(&s2, 1);
        
		/*Если символы различны, то*/
        if (s1 != s2)
			/*Файлы различны*/
            comparisonResult = false;
    }

    return comparisonResult;
}

/*Функция для подсчёта хэша файла*/
unsigned long hashCalculation(QString path) {
    QFile f(path); //передаем файл
    unsigned long hash = 0;

    /*Если файл не получилось открыть, то завершаем работу*/
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;

    char s;

    /*Пока не кончился файл*/
    while (!f.atEnd()) {
		/*Получаем символ*/
        f.read(&s, 1);
		/*Получаем хэш файла*/
        hash += s;
    }

    return hash;
}