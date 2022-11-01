#include <gtest/gtest.h>
#include <glog/logging.h>
#include <memory>
#include <string>
#include <utilities.h>
#include <stacktrace.h>

namespace test {
struct MemoryLeakTest: public ::testing::Test {
};
TEST_F(MemoryLeakTest, testMoveUniqueToSharedPtr){
    google::InstallFailureSignalHandler();
    struct CharBuffer{
        std::string s;
        explicit CharBuffer(const std::string& s):s(s){
            std::cout<<"Ctor"<<std::endl;
        }
        CharBuffer(const CharBuffer& cb):s(cb.s) {
            std::cout<<"Copy ctor"<<std::endl;
        }
        ~CharBuffer() {
            std::cout<<"Dtor"<<std::endl;
        }
        std::unique_ptr<CharBuffer> clone() {
            return std::make_unique<CharBuffer>(s);
        }
    };

    auto cb0 = std::make_shared<CharBuffer>("abc");
    auto cb2 = cb0->clone();
    std::shared_ptr<CharBuffer> cb3 = std::move(cb2);
    std::cout<<cb0.get()<<std::endl;
    std::cout<<cb2.get()<<std::endl;
    std::cout<<cb3.get()<<std::endl;
    DCHECK(cb0.get()==nullptr);
    std::string s;
    //google::glog_internal_namespace_::DumpStackTraceToString(&s);
    std::cout<<s<<std::endl;
}
TEST_F(MemoryLeakTest, testNoCtor) {
    struct A{
      std::vector<std::shared_ptr<std::string>> data;
    };
    auto a = std::make_shared<A>();
    a->data.reserve(10);
    for (int i=0; i < 10; ++i) {
        auto s = std::make_shared<std::string>("aaaaaaaaaa");
        a->data.push_back(s);
    }

    A* a0 = new A(*a.get());
    delete a0;

}
}

int main(int argc, char**argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}