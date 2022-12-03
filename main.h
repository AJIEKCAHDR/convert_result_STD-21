#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <limits>
using namespace std;

#define HEAD_ROW 6

typedef struct {
    double  freq;
    double  value;
} DATA_STD;

typedef struct {
    string  sample;
    string  value;
    string  thickness;
    string  date;
    string  comment;
} NAME_COL_STD;

typedef struct {
    unsigned int  row = 0;
    unsigned int  col = 0;
    unsigned int  max_row = 0;
    unsigned int  num_sample = 0;
    unsigned int  num_value = 0;
} DATA_PARAM;

class ConvertSTD
{
private:
    
public:
    void end_programm(void) {
        cout << "Press ENTER to continue... " << flush;
        cin.ignore(numeric_limits <streamsize> ::max(), '\n' );
        exit(0);
    }

    size_t open_file(ifstream *file, string fileName) {
        if (!file->is_open()) { cout << "Error opening file: '" << fileName << "'." << endl; end_programm(); }

        file->seekg(0, ifstream::end);   // определяем размер файла
        size_t size_file = file->tellg();
        file->seekg(0, ifstream::beg);  // перемещение указателя к началу файла

        cout << endl << "Size file: '" << fileName << "' = " << size_file << " byte" << endl;  // размер файла с результатами

        return size_file;
    }

    int read_file(ifstream *file, DATA_PARAM &p, unsigned int *num_row_cols) {
        string line;
        num_row_cols[0] = 0; 
        
        for (unsigned int last_num_row = 0; getline(*file, line); p.row++) {   // определение колличества строк в файле
            
            if (p.row > HEAD_ROW) {   // пропускать заголовок
                
                if(line.find("Sample=") != string::npos) {   // определить число столбцов в файле
                    num_row_cols[p.col] = p.row - last_num_row;
                    last_num_row = p.row;
                    p.col++;
                }
            } else {
                if(line.find("Sample=") != string::npos) p.col++;
            }   
        }
        if (num_row_cols[1] == 0) { cout << "Error format file!" << endl; end_programm(); }
        
        file->close();

        cout << "Rows = " << p.row << " Сolumns = " << p.col << endl;
        cout << "Rows in 1 column: ";
        num_row_cols[p.col] = num_row_cols[p.col-1];
        for (int i = 1; i <= p.col; i++)  printf("%i ", num_row_cols[i]);
        cout << endl << endl;
        
        for (int i = 0; i < p.col; i++) if (num_row_cols[i] >= p.max_row) p.max_row = num_row_cols[i];   // определить наибольший столбец
        return 1;
    }

    int split_data(ifstream *file, DATA_PARAM &p, unsigned int *num_row_cols, DATA_STD **ds, NAME_COL_STD *name) {

        string line;

        for (int c = 0; c < p.col; c++) {
            for (unsigned int i = num_row_cols[c], r = 0, header_row = HEAD_ROW + num_row_cols[c]; i < num_row_cols[c] + num_row_cols[c+1]-1; i++) {
                
                if (!getline(*file, line)) break;      // построчно читаем файл
                
                if (i > header_row) {
                    // cout << "str: " << line << endl;
                    string line_2 = line.substr(12);      // разделяем частоту и значение по разным строкам
                    
                    line = line.erase(11);
                    // cout << "2str: " << line << " : " << line_2 << endl;
                    
                    ds[c][r].freq = stod(line);         // преобразовываем в число
                    ds[c][r].value = stod(line_2);
                    
                    r++;
                } else {
                    if (i == header_row - HEAD_ROW) name[c].sample = line;  // прочитать имя образца
                    if (i == header_row - (HEAD_ROW-1)) name[c].thickness = line.erase(line.length());  // прочитать толщину образца
                    if (i == header_row - (HEAD_ROW-4)) name[c].date = line.erase(line.length());  // прочитать дату измерения
                    if (i == header_row - (HEAD_ROW-3)) name[c].comment = line;     // прочитать комментарий измерения
                    if (i == header_row) {
                        line.erase(line.length());  // прочитать имя параметра
                        name[c].value = line.erase(line.find(",GHz"), 4);
                    }
                }
            }
            if (!getline(*file, line)) break;
        }

        if (!getline(*file, line)) cout << "Reading file ... OK!" << endl << endl; 
        else { cout << "End of file not reached!" << endl; end_programm(); }

          // подсчёт числа образцов
        for (unsigned int i = 0; i < p.col; i++) if ((name[i].sample != name[i+1].sample) || (name[i].thickness != name[i+1].thickness)) p.num_sample++;
        p.num_value = p.col/p.num_sample;
        cout << "Number of samples = " << p.num_sample << ", Number of value = " << p.num_value << endl << endl;

        return 1;
    }

    int verify_data(DATA_PARAM &p, DATA_STD **ds, NAME_COL_STD *name) {
        
        for (unsigned int i = 0; i < p.col; i+=p.num_value) {
            for (unsigned int n = i; n < i + p.num_value; n++) {
               
                if ((name[n].value.find("Phase") != string::npos) && (n == 0+i)) {
                    
                    string buf_name = name[n].value;
                    name[n].value = name[n+1].value;
                    name[n+1].value = buf_name;

                    DATA_STD *buf_data = new DATA_STD[p.max_row];

                    for (unsigned int r = 0; r < p.max_row-(HEAD_ROW+2); r++) {
                        buf_data[r].freq = ds[n][r].freq;
                        buf_data[r].value = ds[n][r].value;
                        
                        ds[n][r].freq = ds[n+1][r].freq;
                        ds[n][r].value = ds[n+1][r].value;

                        ds[n+1][r].freq = buf_data[r].freq;
                        ds[n+1][r].value = buf_data[r].value;
                    }

                    delete[] buf_data;
                    
                    cout << "Swap column: " << n << "<->" << n+1 << endl; 
                }   
            }
        }
        cout << "Verify data ... OK!" << endl << endl; 
        return 1;
    }

    int save_convert_result(string fileName, DATA_PARAM &p, DATA_STD **ds, NAME_COL_STD *name) {
        
        string fileName_res_sample;

        for (unsigned int i = 0, f_num = 1; i < p.col; i+=p.num_value, f_num++) {
            
            name[i].sample.erase(0, 7);     // удаление перед именем образца 'Sample=''

            fileName_res_sample = name[i].sample.erase(name[i].sample.length()) + "_" + to_string(f_num) + ".asc";    // создаем файл для записи преобразованного результата
            ofstream file_res(fileName_res_sample);
            if (!file_res) { cout << "Error opening file: '" << fileName_res_sample << "'." << endl; end_programm(); }

                for (unsigned int n = i; n < i + p.num_value; n++) {        // запись имен столбцов с данными
                    if (name[n].value.find("E") != string::npos) {          // замена E' -> E1 и E" -> E2
                        if (name[n].value.find("E\"") != string::npos) file_res << name[n].value.replace(name[n].value.find("\""), 1, "2") << "     ";
                        else file_res << name[n].value.replace(name[n].value.find("'"), 1, "1") << "     ";
                    }
                    else file_res << name[n].value << "     ";
                }
                file_res << endl;

                for (unsigned int n = i; n < i + p.num_value; n++) {
                    file_res << "GHz     ";
                    if (name[n].value.find("Phase") != string::npos) file_res << "rad     ";
                    else {
                        if (name[n].value.find("Sg") != string::npos) file_res << "S/cm     ";
                        else file_res << "rel.units     ";
                    }            
                }
                file_res << endl;

                string d_sample = name[i].thickness.substr(name[i].thickness.find('=')+1);
                file_res << name[i].thickness.replace(name[i].thickness.find('='), 6, "     ") << d_sample << "     "  << name[i].date
                 << "     Сreated from a file:'" << fileName <<  "'     Comment=     "  << name[i].comment.erase(0, 8) << endl;

                for (unsigned int r = 0; r < p.max_row-(HEAD_ROW+2); r++) {
                    for (unsigned int c = i; c < i+p.num_value; c++) {
                        file_res << ds[c][r].freq << " " << ds[c][r].value << "     ";
                    }
                    file_res << endl;
                }

            file_res.close();
            cout << name[i].sample << endl; cout << " save ... OK!" << endl;
            fileName_res_sample.clear();
        }
        
        return 1;
    }
};