#include <iostream>
#include <string>
#include <stdexcept>
#include <windows.h>

using namespace std;

template <typename T>
class Array {
private:
    T* data;        // Вказівник на динамічний масив даних
    int capacity;   // Кількість елементів, під які виділено пам'ять (GetSize)
    int count;      // Фактична кількість доданих елементів
    int growBy;     // Крок збільшення пам'яті при переповненні

public:
    // Конструктор за замовчуванням
    Array(int initialSize = 0, int grow = 1)
        : data(nullptr), capacity(0), count(0), growBy(grow > 0 ? grow : 1) {
        if (initialSize > 0) {
            SetSize(initialSize, growBy);
        }
    }

    // Конструктор копіювання (глибоке копіювання)
    Array(const Array<T>& other)
        : data(nullptr), capacity(0), count(0), growBy(other.growBy) {
        if (other.capacity > 0) {
            data = new T[other.capacity]();
            capacity = other.capacity;
            count = other.count;
            for (int i = 0; i < count; ++i) {
                data[i] = other.data[i];
            }
        }
    }

    // Деструктор
    ~Array() {
        RemoveAll();
    }

    // 1. Отримання виділеного розміру масиву
    int GetSize() const {
        return capacity;
    }

    // 2. Встановлення розміру масиву та кроку розширення grow
    void SetSize(int size, int grow = 1) {
        if (size < 0) {
            throw invalid_argument("Розмір масиву не може бути від'ємним.");
        }
        if (grow > 0) {
            growBy = grow;
        }

        if (size == 0) {
            delete[] data;
            data = nullptr;
            capacity = 0;
            count = 0;
            return;
        }

        if (size == capacity) {
            return;
        }

        T* newData = new T[size]();
        int elementsToCopy = (count < size) ? count : size;

        for (int i = 0; i < elementsToCopy; ++i) {
            newData[i] = data[i];
        }

        delete[] data;
        data = newData;
        capacity = size;
        count = elementsToCopy;
    }

    // 3. Отримання останнього допустимого індексу фактичних елементів (-1, якщо порожній)
    int GetUpperBound() const {
        return count - 1;
    }

    // 4. Перевірка, чи порожній масив
    bool IsEmpty() const {
        return count == 0;
    }

    // 5. Видалення «зайвої» виділеної пам'яті вище останнього допустимого індексу
    void FreeExtra() {
        if (count < capacity) {
            SetSize(count, growBy);
        }
    }

    // 6. Повне очищення масиву та звільнення пам'яті
    void RemoveAll() {
        delete[] data;
        data = nullptr;
        capacity = 0;
        count = 0;
    }

    // 7. Отримання елемента за індексом
    T GetAt(int index) const {
        if (index < 0 || index >= count) {
            throw out_of_range("GetAt: індекс виходить за межі доданих елементів.");
        }
        return data[index];
    }

    // 8. Встановлення нового значення для елемента (індекс < GetSize())
    void SetAt(int index, const T& value) {
        if (index < 0 || index >= capacity) {
            throw out_of_range("SetAt: індекс має бути меншим за поточний розмір масиву.");
        }
        data[index] = value;
        if (index >= count) {
            count = index + 1;
        }
    }

    // 9. Оператор індексації [] для запису та читання
    T& operator[](int index) {
        if (index < 0 || index >= capacity) {
            throw out_of_range("operator[]: індекс виходить за межі розміру масиву.");
        }
        if (index >= count) {
            count = index + 1;
        }
        return data[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= count) {
            throw out_of_range("operator[] const: індекс виходить за межі доданих елементів.");
        }
        return data[index];
    }

    // 10. Додавання елемента в кінець масиву (із розширенням на growBy за потреби)
    int Add(const T& value) {
        if (count >= capacity) {
            SetSize(capacity + growBy, growBy);
        }
        data[count] = value;
        return count++;
    }

    // 11. «Складання» (додавання в кінець) елементів іншого масиву
    int Append(const Array<T>& other) {
        if (other.count == 0) {
            return count;
        }

        int otherCount = other.count;
        int totalNeeded = count + otherCount;

        if (totalNeeded > capacity) {
            int newCapacity = capacity;
            while (newCapacity < totalNeeded) {
                newCapacity += growBy;
            }
            SetSize(newCapacity, growBy);
        }

        // Коректно обробляє навіть випадок arr.Append(arr)
        for (int i = 0; i < otherCount; ++i) {
            data[count + i] = other.data[i];
        }
        count += otherCount;
        return count;
    }

    // 12. Оператор присвоєння копіюванням
    Array<T>& operator=(const Array<T>& other) {
        if (this == &other) {
            return *this;
        }

        delete[] data;
        data = nullptr;
        capacity = 0;
        count = 0;
        growBy = other.growBy;

        if (other.capacity > 0) {
            data = new T[other.capacity]();
            capacity = other.capacity;
            count = other.count;
            for (int i = 0; i < count; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    // 13. Отримання адреси внутрішнього масиву з даними
    T* GetData() {
        return data;
    }

    const T* GetData() const {
        return data;
    }

    // 14. Вставлення одного або кількох однакових елементів у задану позицію
    void InsertAt(int index, const T& value, int insertCount = 1) {
        if (index < 0 || index > count) {
            throw out_of_range("InsertAt: некоректна позиція для вставки.");
        }
        if (insertCount <= 0) {
            return;
        }

        int totalNeeded = count + insertCount;
        if (totalNeeded > capacity) {
            int newCapacity = capacity;
            while (newCapacity < totalNeeded) {
                newCapacity += growBy;
            }
            SetSize(newCapacity, growBy);
        }

        // Зсув наявних елементів праворуч
        for (int i = count - 1; i >= index; --i) {
            data[i + insertCount] = data[i];
        }

        // Заповнення звільнених позицій
        for (int i = 0; i < insertCount; ++i) {
            data[index + i] = value;
        }
        count += insertCount;
    }

    // Перевантаження InsertAt для вставки іншого масиву в задану позицію
    void InsertAt(int index, const Array<T>& newArray) {
        if (index < 0 || index > count) {
            throw out_of_range("InsertAt: некоректна позиція для вставки масиву.");
        }
        if (newArray.count == 0) {
            return;
        }

        if (this == &newArray) {
            Array<T> temp(newArray);
            InsertAt(index, temp);
            return;
        }

        int insertCount = newArray.count;
        int totalNeeded = count + insertCount;
        if (totalNeeded > capacity) {
            int newCapacity = capacity;
            while (newCapacity < totalNeeded) {
                newCapacity += growBy;
            }
            SetSize(newCapacity, growBy);
        }

        for (int i = count - 1; i >= index; --i) {
            data[i + insertCount] = data[i];
        }

        for (int i = 0; i < insertCount; ++i) {
            data[index + i] = newArray.data[i];
        }
        count += insertCount;
    }

    // 15. Видалення одного або кількох елементів із заданої позиції
    void RemoveAt(int index, int removeCount = 1) {
        if (index < 0 || index >= count || removeCount <= 0 || index + removeCount > count) {
            throw out_of_range("RemoveAt: некоректний індекс або кількість елементів для видалення.");
        }

        for (int i = index + removeCount; i < count; ++i) {
            data[i - removeCount] = data[i];
        }
        count -= removeCount;
    }
};

// Допоміжна шаблонна функція для виведення стану контейнера
template <typename T>
void printArrayState(const Array<T>& arr, const string& title) {
    cout << title << "\n";
    cout << "  IsEmpty: " << (arr.IsEmpty() ? "так" : "ні")
         << " | GetSize (пам'ять): " << arr.GetSize()
         << " | GetUpperBound (ост. індекс): " << arr.GetUpperBound()
         << " | GetData(): " << arr.GetData() << "\n  Елементи: [ ";

    for (int i = 0; i <= arr.GetUpperBound(); ++i) {
        cout << arr[i] << (i < arr.GetUpperBound() ? ", " : " ");
    }
    cout << "]\n\n";
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    try {
        // =========================================================
        // 1. Тестування Array<int> (логіка SetSize(5, 5), Add, FreeExtra)
        // =========================================================
        Array<int> arr1;
        printArrayState(arr1, "1. Початковий стан arr1:");

        arr1.SetSize(5, 5);
        for (int i = 1; i <= 4; ++i) {
            arr1.Add(i * 10);
        }
        printArrayState(arr1, "2. Після SetSize(5, 5) та додавання 4 елементів (UpperBound має бути 3):");

        // Додаємо 5-й і 6-й елементи (розмір має стрибнути з 5 до 10)
        arr1.Add(50);
        arr1.Add(60);
        printArrayState(arr1, "3. Після додавання 5-го та 6-го елементів (розмір збільшився на grow=5 до 10):");

        // Перевірка SetAt, GetAt та operator[]
        arr1.SetAt(0, 100);
        arr1[1] = 200;
        cout << "4. Перевірка GetAt(0) = " << arr1.GetAt(0)
             << ", arr1[1] = " << arr1[1] << "\n\n";

        // Перевірка FreeExtra
        arr1.FreeExtra();
        printArrayState(arr1, "5. Після FreeExtra() (зайва пам'ять вище індексу 5 звільнена):");

        // Перевірка InsertAt та RemoveAt
        arr1.InsertAt(2, 999, 2); // Вставляємо два числа 999 на позицію 2
        printArrayState(arr1, "6. Після InsertAt(2, 999, 2):");

        arr1.RemoveAt(2, 2); // Видаляємо ці два числа
        printArrayState(arr1, "7. Після RemoveAt(2, 2):");

        // Перевірка operator= та Append
        Array<int> arr2;
        arr2 = arr1;
        arr1.Append(arr2);
        printArrayState(arr1, "8. Після arr2 = arr1 та arr1.Append(arr2):");

        // Перевірка RemoveAll
        arr1.RemoveAll();
        printArrayState(arr1, "9. Після RemoveAll():");

        // =========================================================
        // 2. Тестування шаблонного класу з типом std::string
        // =========================================================
        Array<string> words(2, 2);
        words.Add("C++");
        words.Add("Templates");
        words.InsertAt(1, "Class");
        printArrayState(words, "10. Тестування Array<string>:");

    } catch (const exception& ex) {
        cout << "Виняток: " << ex.what() << endl;
    }

    return 0;
}