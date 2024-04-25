#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    size_t capacity_;

    ReserveProxyObj(size_t capacity) {
        capacity_ = capacity;
    }
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : 
        array_ptr_(size),
        capacity_(size),
        size_(size) {
        for(auto it = begin(); it != end(); ++it) {
            *it = std::move(Type{});
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : 
        array_ptr_(size),
        capacity_(size),
        size_(size) {
        for(auto it = begin(); it != end(); ++it) {
            *it = std::move(value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :
        array_ptr_(init.size()),
        capacity_(init.size()),
        size_(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        // Напишите тело самостоятельно
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >= GetSize()) {
            throw std::out_of_range("Index is out of range");
        }
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >= GetSize()) {
            throw std::out_of_range("Index is out of range");
        }
        return array_ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size < GetCapacity()) {
            if(new_size > size_) {
                for(auto it = std::next(begin() + GetSize() - 1); it != std::next(begin() + new_size); ++it) {
                    *it = std::move(Type{});
                }
            }
            size_ = new_size;
        } else {
            size_t new_capacity = std::max(new_size, GetCapacity() * 2);
            SimpleVector new_array(new_capacity);
            std::move(begin(), end(), new_array.begin());
            swap(new_array);
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &array_ptr_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return &array_ptr_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return &array_ptr_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return &array_ptr_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return &array_ptr_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return &array_ptr_[size_];
    }

    SimpleVector(const SimpleVector& other) :
        array_ptr_(other.GetSize()),
        capacity_(other.GetSize()),
        size_(other.GetSize()) {
        std::move(other.begin(), other.end(), begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if(this != &rhs) {
            auto vector_copy(rhs);
            swap(vector_copy);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if(capacity_ == 0) {
            ResizeCapacity(1);
        } else if(size_ == capacity_) {
            ResizeCapacity(GetCapacity() * 2);
        }
        array_ptr_[size_] = std::move(item);
        ++size_;
    }
    
    void PushBack(Type&& item) {
        if(capacity_ == 0) {
            ResizeCapacity(1);
        } else if(size_ == capacity_) {
            ResizeCapacity(GetCapacity() * 2);
        }
        array_ptr_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t pos_distance = static_cast<size_t>(std::distance(cbegin(), pos));
        if(capacity_ == 0) {
            ResizeCapacity(1);
        } else if(size_ == capacity_) {
            ResizeCapacity(capacity_ * 2);
        }
        std::move_backward(std::next(begin(), pos_distance), end(), end() + 1);
        array_ptr_[pos_distance] = std::move(value);
        ++size_;
        return &array_ptr_[pos_distance];
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t pos_distance = static_cast<size_t>(std::distance(cbegin(), pos));
        if(capacity_ == 0) {
            ResizeCapacity(1);
        } else if(size_ == capacity_) {
            ResizeCapacity(capacity_ * 2);
        }
        std::move_backward(begin() + pos_distance, end(), end() + 1);
        array_ptr_[pos_distance] = std::move(value);
        ++size_;
        return &array_ptr_[pos_distance];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if(size_ > 0) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        // Напишите тело самостоятельно
        size_t pos_distance = static_cast<size_t>(std::distance(cbegin(), pos));
        std::move(begin() + pos_distance + 1, end(), begin() + pos_distance);
        PopBack();
        return &array_ptr_[pos_distance];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        array_ptr_.swap(other.array_ptr_);
        std::swap(other.capacity_, capacity_);
        std::swap(other.size_, size_);
    }

    SimpleVector(ReserveProxyObj reserve) : 
        SimpleVector(reserve.capacity_) {
        size_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (capacity_ >= new_capacity) {
            return;
        }
        ResizeCapacity(new_capacity);
    }

    SimpleVector(SimpleVector&& other) {
        array_ptr_ = std::move(other.array_ptr_);
        capacity_ = std::exchange(other.capacity_, 0);
        size_ = std::exchange(other.size_, 0);
    }

    SimpleVector& operator=(SimpleVector&& other) {
        if(this != &other) {
            array_ptr_ = std::move(other.array_ptr_);
            capacity_ = std::exchange(other.capacity_, 0);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

private:
    ArrayPtr<Type> array_ptr_;
    size_t capacity_ = 0;
    size_t size_ = 0;

    void ResizeCapacity(size_t new_capacity) {
        SimpleVector new_array(new_capacity);
        std::move(begin(), end(), new_array.begin());
        array_ptr_.swap(new_array.array_ptr_);
        std::swap(new_array.capacity_, capacity_);
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if(lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs == lhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};