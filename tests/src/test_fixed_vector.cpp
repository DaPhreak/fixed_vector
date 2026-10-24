#include "fixed_vector/fixed_vector.h"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

TEST_CASE("Test fixed_vector", "[vector]")
{
    const std::string TestStr{ "Hello world!" };
    const std::string_view TestView{ TestStr };

    using Vec = phreak::fixed_vector< std::string, 5 >;
    Vec A;

    REQUIRE( A.empty() );
    REQUIRE( A.size() == 0 );
    A.push_back( TestStr );
    REQUIRE_FALSE( A.empty() );
    REQUIRE( A.size() == 1 );
    REQUIRE( (A.at( 0 ) == TestStr) );
    REQUIRE( (A[ 0 ] == TestStr) );
    {
        Vec B{A};

        REQUIRE( A == B );
        B.emplace_back( TestView );
        A=B;
    }
    REQUIRE( A.size() == 2 );
    REQUIRE( (A.at( 1 ) == TestStr) );
    {
        Vec B{ std::move(A) };
        B.emplace_back( TestView );
        A=std::move(B);
    }
    for ( auto&& v: A ) {
        REQUIRE( (v == TestStr) );
    }

    A.emplace_back("Jooo!");
    REQUIRE( A.size() == 4 );
    REQUIRE( (A.back() == "Jooo!") );
    A.erase( A.begin() + 1 );
    REQUIRE( A.size() == 3 );
    REQUIRE( (A.back() == "Jooo!") );
    A.erase( A.begin(), A.begin() + 2 );
    REQUIRE( A.size() == 1 );
    REQUIRE( (A.back() == "Jooo!") );
    A.erase( A.begin() );
    REQUIRE( A.size() == 0 );
    A.emplace_back("1");
    A.emplace_back("2");
    A.emplace_back("3");
    A.emplace_back("4");
    A.emplace_back("5");
    A.erase( A.begin() + 1, A.begin() + 3 );
    REQUIRE( A.size() == 3 );
    REQUIRE( (A.front() == "1") );
    REQUIRE( (A[ 1 ] == "4") );
    REQUIRE( (A.back() == "5") );
    A.erase( A.end() - 1 );
    REQUIRE( A.size() == 2 );
    REQUIRE( (A.front() == "1") );
    REQUIRE( (A.back() == "4") );
    std::vector<int> iVec{1,2,3,4};
    iVec.emplace(iVec.end(), 55);
    A.clear();
    REQUIRE( A.empty() );
    A.emplace( A.begin(), "2" );
    REQUIRE( (A.front() == "2") );
    A.emplace( A.begin(), "1" );
    A.emplace( A.end(), "5" );
    REQUIRE( (A.front() == "1") );
    REQUIRE( (A.back() == "5") );
    A.emplace( A.begin() + 2 , "4" );
    A.emplace( A.begin() + 2 , "3" );
    REQUIRE( A.size() == 5 );
    for ( size_t I{}; I < A.size(); ++I ) {
        REQUIRE( (A[I] == std::to_string( I + 1 ) ) );
    }
    A.resize(5);
    REQUIRE( A.size() == 5 );
    A.resize(4);
    REQUIRE( A.size() == 4 );
    A.resize(5, "5" );
    REQUIRE( A.size() == 5 );
    for ( size_t I{}; I < A.size(); ++I ) {
        REQUIRE( (A[I] == std::to_string( I + 1 ) ) );
    }
    {
        A.clear();
        std::string s( "Test" );
        A.resize(5, std::move( s ) );
        REQUIRE( A.size() == 5 );
        for ( size_t I{}; I < A.size(); ++I ) {
            REQUIRE( (A[I] == "Test" ) );
        }
        A.resize(2);
        A.resize(5, std::string{"Test"} );
        REQUIRE( A.size() == 5 );
        for ( size_t I{}; I < A.size(); ++I ) {
            REQUIRE( (A[I] == "Test" ) );
        }
        REQUIRE( Vec{5,"Test"}.size() == 5 );
        REQUIRE( (Vec{5,"Test"}.front() == "Test") );
    }
    {
        A.resize(2);
        std::string s( "TestAssign" );
        A.assign(5, std::move( s ) );
        REQUIRE( A.size() == 5 );
        for ( size_t I{}; I < A.size(); ++I ) {
            REQUIRE( (A[I] == "TestAssign" ) );
        }
    }
    {
        A.resize(2);
        A.assign({ "1", "2", "3", "4", "5" });
        REQUIRE( A.size() == 5 );
        for ( size_t I{}; I < A.size(); ++I ) {
            REQUIRE( (A[I] == std::to_string( I + 1 ) ) );
        }
        Vec B{};
        B.resize(3);
        B=A;
        REQUIRE ( A == B );
        REQUIRE ( !(A != B) );
    }

    {
        Vec B{};

        REQUIRE_THROWS( B.reserve( Vec::max_size() + 1 ) );
        REQUIRE_THROWS( B.resize( Vec::max_size() + 1 ) );
        B.resize( Vec::max_size() - 1, std::string{ "B" } );
        REQUIRE( B.try_emplace_back() );
        REQUIRE_THROWS( B.emplace_back() );
        REQUIRE( ! B.try_emplace_back() );
        B.pop_back();
        REQUIRE( B.try_push_back( "Jo" ) );
        REQUIRE_THROWS( B.push_back( "No" ) );
        REQUIRE( ! B.try_push_back( "No" ) );

        std::swap( A, B );
        REQUIRE( A.size() == Vec::max_size() );
        REQUIRE( A.back() == "Jo" );
        REQUIRE( B.back() == "5" );
        std::swap( A, B );
        REQUIRE( B.size() == Vec::max_size() );
        REQUIRE( B.back() == "Jo" );
        REQUIRE( A.back() == "5" );
    }
    {
        Vec B{};
        B = { "1", "2", "3", "4", "5" };
        REQUIRE( B.size() == 5 );
        for ( size_t I{}; I < B.size(); ++I ) {
            REQUIRE( ( B[I] == std::to_string( I + 1 ) ) );
        }
        B = { "1", "2", "3" };
        REQUIRE( B.size() == 3 );
        for ( size_t I{}; I < B.size(); ++I ) {
            REQUIRE( ( B[I] == std::to_string( I + 1 ) ) );
        }
    }
}

} // ::