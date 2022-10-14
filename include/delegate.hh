// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/1/9.
//

#ifndef CPP_ETUDES_INCLUDE_DELEGATE_HH
#define CPP_ETUDES_INCLUDE_DELEGATE_HH
#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <vector>
class ReaderObj;
class AbsReader {
public:
    virtual size_t size() { return 0; }
    virtual size_t readByte(int8_t& byte) { return 0; }
    virtual size_t readBytes(std::vector<int8_t>& bytes, size_t n) { return 0; }

    virtual ~AbsReader() {}
    friend class ReaderObj;

protected:
    virtual AbsReader& get_delegate() = 0;
};

using DeleteFunc = std::function<void(AbsReader*)>;

class ReaderObj {
public:
    ReaderObj(AbsReader* reader, DeleteFunc deleter) : reader(reader), deleter(deleter) {}
    ~ReaderObj() { deleter(reader); }
    AbsReader& operator*() { return reader->get_delegate(); }
    AbsReader* operator->() { return &reader->get_delegate(); }

private:
    ReaderObj(const ReaderObj&) = delete;
    ReaderObj(ReaderObj&&) = delete;
    ReaderObj& operator=(const ReaderObj&) = delete;
    ReaderObj& operator=(ReaderObj&&) = delete;
    AbsReader* reader;
    int _scale = 0;
    DeleteFunc deleter;
};

class DelegateReader final : public AbsReader {
public:
    size_t size() override { return 100; }
    size_t readByte(int8_t& byte) override {
        byte = 'x';
        return 1;
    }
    size_t readBytes(std::vector<int8_t>& bytes, size_t n) override {
        bytes.resize(n);
        for (auto i = 0; i < n; ++i) {
            bytes[i] = 'X';
        }
        return n;
    }

protected:
    AbsReader& get_delegate() override { return *this; }
};

class TypeAReader final : public AbsReader {
public:
    TypeAReader(AbsReader* delegate, DeleteFunc deleter) : delegate(delegate), deleter(deleter) {}

    ~TypeAReader() override { deleter(delegate); }

protected:
    AbsReader& get_delegate() override { return *delegate; }

private:
    AbsReader* delegate;
    DeleteFunc deleter;
};

class TypeBReader final : public AbsReader {
public:
    size_t size() override { return 99; }
    size_t readByte(int8_t& byte) override {
        byte = 'b';
        return 1;
    }
    size_t readBytes(std::vector<int8_t>& bytes, size_t n) override {
        bytes.resize(n);
        for (auto i = 0; i < n; ++i) {
            bytes[i] = 'B';
        }
        return n;
    }

protected:
    AbsReader& get_delegate() override { return *this; }
};

#endif // CPP_ETUDES_INCLUDE_DELEGATE_HH
