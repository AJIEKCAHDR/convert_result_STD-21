#include "main.h"

int main(void) {
	
    ConvertSTD std21;

    cout << "Enter path file result STD-21: ";

    string fileName_std;// = "asa_test.asc";
    cin >> fileName_std;               // ввод с кливиатуры пути к файлу с результатами 
    ifstream file_std(fileName_std);    
    
    std21.open_file(&file_std, fileName_std); // открываем файл

    DATA_PARAM pd;
    unsigned int *row_in_col = new unsigned int[100];

    std21.read_file(&file_std, pd, row_in_col); // чтение файла

    file_std.close();                           // закрытие файла

	DATA_STD **data = new DATA_STD*[pd.col+1];  // выделение памяти для данных
	for (unsigned int i = 0; i <= pd.col; i++) data[i] = new DATA_STD[pd.max_row];
    NAME_COL_STD *col_Name = new NAME_COL_STD[pd.col+1];

    file_std.open(fileName_std);                // повторное открытие файла

    std21.split_data(&file_std, pd, row_in_col, data, col_Name);  // разделение данных

    std21.verify_data(pd, data, col_Name);      // проверка и перенос столбцов

    std21.save_convert_result(fileName_std, pd, data, col_Name);  // сохранение данных в преобразованном формате

    delete[] row_in_col;
    for (unsigned int i = 0; i <= pd.col; i++) delete[] data[i];
    delete[] data;
    delete[] col_Name;
    
    std21.end_programm();
}