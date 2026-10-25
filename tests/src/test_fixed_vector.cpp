#include "fixed_vector/fixed_vector.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

using IVec = phreak::fixed_vector<int, 4>;
using SVec = phreak::fixed_vector<std::string, 5>;

struct Tracked {
    static inline int alive      = 0;
    static inline int throwAfter = -1;

    int value;

    explicit Tracked(int v) : value(v) { tick(); ++alive; }
    Tracked(const Tracked& o) : value(o.value) { tick(); ++alive; }
    Tracked(Tracked&& o) : value(o.value) { tick(); ++alive; }
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) = default;
    ~Tracked() { --alive; }

    friend bool operator==(const Tracked& a, const Tracked& b) { return a.value == b.value; }

    static void tick()
    {
        if (throwAfter == 0) {
            throwAfter = -1;
            throw std::runtime_error("Tracked: armed construction");
        }
        if (throwAfter > 0) {
            --throwAfter;
        }
    }
};

using TVec = phreak::fixed_vector<Tracked, 8>;

struct Counted {
    static inline int copies = 0;

    std::string s;

    Counted(const char* p) : s(p) {}
    Counted(const Counted& o) : s(o.s) { ++copies; }
    Counted(Counted&&) noexcept = default;
    Counted& operator=(const Counted& o) { s = o.s; ++copies; return *this; }
    Counted& operator=(Counted&&) noexcept = default;
};

struct NoDefault {
    explicit NoDefault(int v) : value(v) {}
    int value;
};

struct alignas(64) OverAligned {
    int value;
};

static_assert(std::is_same_v<IVec::value_type, int>);
static_assert(std::is_same_v<IVec::iterator, int*>);
static_assert(std::is_same_v<IVec::const_iterator, const int*>);
static_assert(IVec::capacity() == 4);
static_assert(IVec::max_size() == 4);
static_assert(sizeof(phreak::fixed_vector<char, 15>) == 16);

static_assert(std::is_nothrow_default_constructible_v<IVec>);
static_assert(std::is_nothrow_copy_constructible_v<IVec>);
static_assert(std::is_nothrow_move_constructible_v<IVec>);
static_assert(std::is_nothrow_copy_assignable_v<IVec>);
static_assert(std::is_nothrow_move_assignable_v<IVec>);
static_assert(std::is_nothrow_swappable_v<IVec>);

static_assert(!std::is_nothrow_move_constructible_v<TVec>);
static_assert(!std::is_nothrow_swappable_v<TVec>);

static_assert(alignof(phreak::fixed_vector<OverAligned, 2>) >= alignof(OverAligned));

static_assert(!std::is_convertible_v<std::size_t, IVec>);

static_assert(!std::is_constructible_v<phreak::fixed_vector<std::unique_ptr<int>, 2>,
                                       std::initializer_list<int*>>);
static_assert(!std::is_constructible_v<phreak::fixed_vector<std::vector<int>, 2>,
                                       std::initializer_list<int>>);

// TODO: enable once the initializer_list<U> overload rejects narrowing (is_list_convertible).
// static_assert(!std::is_constructible_v<IVec, std::initializer_list<double>>);

TEST_CASE("default construction", "[ctor]")
{
    const IVec v;
    REQUIRE(v.empty());
    REQUIRE_FALSE(v.full());
    REQUIRE(v.size() == 0);
    REQUIRE(v.begin() == v.end());
    REQUIRE(v.cbegin() == v.cend());
    REQUIRE(v.rbegin() == v.rend());
    REQUIRE(v.data() == v.begin());
}

TEST_CASE("count constructor", "[ctor]")
{
    const IVec zeros(3);
    REQUIRE(zeros.size() == 3);
    REQUIRE(std::all_of(zeros.begin(), zeros.end(), [](int i) { return i == 0; }));

    const SVec filled(4, "x");
    REQUIRE(filled.size() == 4);
    REQUIRE(std::all_of(filled.begin(), filled.end(), [](const std::string& s) { return s == "x"; }));

    const SVec multi(2, 3, 'a');
    REQUIRE(multi.size() == 2);
    REQUIRE(multi[0] == "aaa");
    REQUIRE(multi[1] == "aaa");

    std::string s{"moved"};
    const SVec fromRvalue(3, std::move(s));
    REQUIRE(fromRvalue.size() == 3);
    REQUIRE(std::all_of(fromRvalue.begin(), fromRvalue.end(), [](const std::string& e) { return e == "moved"; }));

    REQUIRE_THROWS_AS(IVec(5), std::bad_alloc);
}

TEST_CASE("range constructor", "[ctor]")
{
    const std::vector<int> src{1, 2, 3};
    const IVec fromForward(src.begin(), src.end());
    REQUIRE(fromForward.size() == 3);
    REQUIRE(std::equal(fromForward.begin(), fromForward.end(), src.begin(), src.end()));

    std::istringstream in{"4 5 6"};
    const IVec fromInput(std::istream_iterator<int>{in}, std::istream_iterator<int>{});
    const IVec expected{4, 5, 6};
    REQUIRE(fromInput == expected);

    const int raw[] = {7, 8};
    const IVec fromPointers(std::begin(raw), std::end(raw));
    REQUIRE(fromPointers.size() == 2);
    REQUIRE(fromPointers.back() == 8);

    const std::vector<int> tooMany{1, 2, 3, 4, 5};
    REQUIRE_THROWS_AS(IVec(tooMany.begin(), tooMany.end()), std::bad_alloc);

    std::istringstream tooManyIn{"1 2 3 4 5"};
    REQUIRE_THROWS_AS(IVec(std::istream_iterator<int>{tooManyIn}, std::istream_iterator<int>{}), std::bad_alloc);
}

TEST_CASE("initializer_list constructor", "[ctor]")
{
    const IVec ints{1, 2, 3};
    REQUIRE(ints.size() == 3);
    REQUIRE(ints.front() == 1);
    REQUIRE(ints.back() == 3);

    const SVec strings{"a", "bb", "ccc"};
    REQUIRE(strings.size() == 3);
    REQUIRE(strings[1] == "bb");

    const phreak::fixed_vector<long, 8> mixed{3, 7L};
    REQUIRE(mixed.size() == 2);
    REQUIRE(mixed[0] == 3);
    REQUIRE(mixed[1] == 7);

    REQUIRE_THROWS_AS((IVec{1, 2, 3, 4, 5}), std::bad_alloc);
}

TEST_CASE("initializer_list<U> constructs elements in place", "[ctor][design]")
{
    Counted::copies = 0;
    const phreak::fixed_vector<Counted, 4> v{"one", "two", "three"};
    REQUIRE(v.size() == 3);
    REQUIRE(v[2].s == "three");
    REQUIRE(Counted::copies == 0);
}

TEST_CASE("copy construction and assignment", "[copy]")
{
    const SVec a{"1", "2", "3"};

    SVec b{a};
    REQUIRE(b == a);
    b[0] = "changed";
    REQUIRE(a[0] == "1");

    SVec shorter{"x"};
    shorter = a;
    REQUIRE(shorter == a);

    SVec longer{"a", "b", "c", "d", "e"};
    longer = a;
    REQUIRE(longer == a);

    SVec& alias = longer;
    longer = alias;
    REQUIRE(longer == a);
}

TEST_CASE("move construction and assignment", "[move]")
{
    SVec src{"1", "2", "3"};
    const SVec expected{src};

    SVec moved{std::move(src)};
    REQUIRE(moved == expected);
    REQUIRE(src.empty());

    SVec target{"a", "b", "c", "d", "e"};
    target = std::move(moved);
    REQUIRE(target == expected);
    REQUIRE(moved.empty());

    SVec& alias = target;
    target = std::move(alias);
    REQUIRE(target.size() <= expected.size());

    phreak::fixed_vector<std::unique_ptr<int>, 4> owners;
    owners.emplace_back(std::make_unique<int>(42));
    auto newOwner{std::move(owners)};
    REQUIRE(newOwner.size() == 1);
    REQUIRE(*newOwner.front() == 42);
    REQUIRE(owners.empty());
}

TEST_CASE("element access", "[access]")
{
    IVec v{10, 20, 30};
    const IVec& cv = v;

    REQUIRE(v.front() == 10);
    REQUIRE(v.back() == 30);
    REQUIRE(v[1] == 20);
    REQUIRE(v.at(2) == 30);
    REQUIRE(cv.front() == 10);
    REQUIRE(cv.back() == 30);
    REQUIRE(cv[1] == 20);
    REQUIRE(cv.at(2) == 30);
    REQUIRE(*v.data() == 10);
    REQUIRE(*cv.data() == 10);

    v.front() = 1;
    v[1]      = 2;
    v.at(2)   = 3;
    const IVec expected{1, 2, 3};
    REQUIRE(v == expected);

    REQUIRE_THROWS_AS(v.at(3), std::out_of_range);
    REQUIRE_THROWS_AS(cv.at(100), std::out_of_range);
}

TEST_CASE("iteration", "[iterator]")
{
    IVec v{3, 1, 4, 2};

    REQUIRE(std::accumulate(v.begin(), v.end(), 0) == 10);
    REQUIRE(std::distance(v.begin(), v.end()) == 4);

    const std::vector<int> reversed(v.rbegin(), v.rend());
    REQUIRE(reversed == std::vector<int>{2, 4, 1, 3});

    const std::vector<int> creversed(v.crbegin(), v.crend());
    REQUIRE(creversed == reversed);

    std::sort(v.begin(), v.end());
    const IVec sorted{1, 2, 3, 4};
    REQUIRE(v == sorted);

    int sum = 0;
    for (int i : v) {
        sum += i;
    }
    REQUIRE(sum == 10);
}

TEST_CASE("capacity queries and reserve", "[capacity]")
{
    IVec v{1, 2, 3, 4};
    REQUIRE(v.full());
    REQUIRE(v.size() == v.capacity());

    REQUIRE_NOTHROW(IVec::reserve(4));
    REQUIRE_THROWS_AS(IVec::reserve(5), std::bad_alloc);
    IVec::shrink_to_fit();
    REQUIRE(v.size() == 4);
}

TEST_CASE("push_back / emplace_back and their variants", "[modifiers]")
{
    SVec v;
    const std::string lvalue{"lvalue"};

    std::string& a = v.push_back(lvalue);
    REQUIRE(a == "lvalue");
    REQUIRE(&a == &v.back());

    std::string& b = v.push_back(std::string{"rvalue"});
    REQUIRE(b == "rvalue");

    std::string& c = v.emplace_back(3, 'z');
    REQUIRE(c == "zzz");

    std::string& d = v.unchecked_emplace_back("unchecked");
    REQUIRE(d == "unchecked");

    std::string* e = v.try_emplace_back("try");
    REQUIRE(e != nullptr);
    REQUIRE(*e == "try");
    REQUIRE(v.full());

    REQUIRE(v.try_emplace_back("no") == nullptr);
    REQUIRE(v.try_push_back(lvalue) == nullptr);
    REQUIRE(v.try_push_back(std::string{"no"}) == nullptr);
    REQUIRE_THROWS_AS(v.emplace_back("no"), std::bad_alloc);
    REQUIRE_THROWS_AS(v.push_back(lvalue), std::bad_alloc);
    REQUIRE_THROWS_AS(v.push_back(std::string{"no"}), std::bad_alloc);
    REQUIRE(v.size() == 5);
    REQUIRE(v.back() == "try");

    v.pop_back();
    REQUIRE(v.unchecked_push_back(lvalue) == "lvalue");
    v.pop_back();
    REQUIRE(v.unchecked_push_back(std::string{"moved"}) == "moved");
}

TEST_CASE("pop_back and clear", "[modifiers]")
{
    SVec v{"1", "2", "3"};
    v.pop_back();
    REQUIRE(v.size() == 2);
    REQUIRE(v.back() == "2");

    v.clear();
    REQUIRE(v.empty());
    v.clear();
    REQUIRE(v.empty());

    v.emplace_back("reuse");
    REQUIRE(v.front() == "reuse");
}

TEST_CASE("resize", "[modifiers]")
{
    SVec v{"1", "2"};

    v.resize(4);
    REQUIRE(v.size() == 4);
    REQUIRE(v[2].empty());
    REQUIRE(v[3].empty());

    v.resize(1);
    REQUIRE(v.size() == 1);
    REQUIRE(v[0] == "1");

    v.resize(3, "fill");
    REQUIRE(v.size() == 3);
    REQUIRE(v[1] == "fill");
    REQUIRE(v[2] == "fill");

    v.resize(5, 2, 'q');
    REQUIRE(v[4] == "qq");

    v.resize(5, "ignored");
    REQUIRE(v[4] == "qq");

    REQUIRE_THROWS_AS(v.resize(6), std::bad_alloc);
    REQUIRE(v.size() == 5);

    v.resize(2);
    REQUIRE(v.size() == 2);
    v.resize(3, "u");
    REQUIRE(v.back() == "u");
}

TEST_CASE("assign(count, args...)", "[assign]")
{
    SVec v{"a", "b"};

    v.assign(4, "x");
    REQUIRE(v.size() == 4);
    REQUIRE(std::all_of(v.begin(), v.end(), [](const std::string& s) { return s == "x"; }));

    v.assign(1, "y");
    REQUIRE(v.size() == 1);
    REQUIRE(v[0] == "y");

    v.assign(3, 2, 'k');
    REQUIRE(v.size() == 3);
    REQUIRE(v[2] == "kk");

    v.assign(0, "z");
    REQUIRE(v.empty());

    REQUIRE_THROWS_AS(v.assign(6, "z"), std::bad_alloc);
}

TEST_CASE("assign(count, value) where value aliases an element", "[assign][aliasing]")
{
    SVec v{"0", "1", "2", "3", "4"};
    v.assign(2, v[4]);
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == "4");
    REQUIRE(v[1] == "4");
}

TEST_CASE("assign from ranges", "[assign]")
{
    IVec v{9, 9, 9};

    const std::vector<int> two{1, 2};
    v.assign(two.begin(), two.end());
    const IVec expectTwo{1, 2};
    REQUIRE(v == expectTwo);

    const std::vector<int> four{1, 2, 3, 4};
    v.assign(four.begin(), four.end());
    const IVec expectFour{1, 2, 3, 4};
    REQUIRE(v == expectFour);

    std::istringstream in{"7 8 9"};
    v.assign(std::istream_iterator<int>{in}, std::istream_iterator<int>{});
    const IVec expectInput{7, 8, 9};
    REQUIRE(v == expectInput);

    v.assign(two.end(), two.end());
    REQUIRE(v.empty());

    const std::vector<const char*> words{"x", "y"};
    SVec s{"old", "old", "old"};
    s.assign(words.begin(), words.end());
    REQUIRE(s.size() == 2);
    REQUIRE(s[1] == "y");
}

TEST_CASE("assign overflow from forward range leaves vector unchanged", "[assign][exception]")
{
    IVec v{1, 2, 3};
    const IVec before{v};
    const std::vector<int> tooMany{4, 5, 6, 7, 8};

    REQUIRE_THROWS_AS(v.assign(tooMany.begin(), tooMany.end()), std::bad_alloc);
    REQUIRE(v == before);
}

TEST_CASE("assign overflow from input range keeps a valid state", "[assign][exception]")
{
    IVec v{1, 2, 3};
    std::istringstream in{"4 5 6 7 8"};

    REQUIRE_THROWS_AS(v.assign(std::istream_iterator<int>{in}, std::istream_iterator<int>{}), std::bad_alloc);
    REQUIRE(v.size() == v.capacity());
}

TEST_CASE("assign from an overlapping range of the same vector", "[assign][aliasing]")
{
    IVec v{1, 2, 3, 4};
    v.assign(v.begin() + 1, v.end());
    const IVec expected{2, 3, 4};
    REQUIRE(v == expected);
}

TEST_CASE("assign / operator= from initializer_list", "[assign]")
{
    SVec v{"a"};

    v.assign({"1", "2", "3"});
    const SVec expected{"1", "2", "3"};
    REQUIRE(v == expected);

    v = {"x", "y"};
    REQUIRE(v.size() == 2);
    REQUIRE(v[1] == "y");

    v = {std::string{"s"}, "mixed"};
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == "s");

    v = {};
    REQUIRE(v.empty());

    REQUIRE_THROWS_AS((v = {"1", "2", "3", "4", "5", "6"}), std::bad_alloc);
}

TEST_CASE("emplace / insert", "[insert]")
{
    SVec v;

    auto it = v.emplace(v.end(), "c");
    REQUIRE(*it == "c");
    it = v.emplace(v.begin(), "a");
    REQUIRE(it == v.begin());
    it = v.emplace(v.begin() + 1, "b");
    REQUIRE(it == v.begin() + 1);
    REQUIRE(*it == "b");

    const std::string e{"e"};
    it = v.insert(v.end(), e);
    REQUIRE(*it == "e");
    it = v.insert(v.begin() + 3, std::string{"d"});
    REQUIRE(*it == "d");

    const SVec expected{"a", "b", "c", "d", "e"};
    REQUIRE(v == expected);

    REQUIRE_THROWS_AS(v.emplace(v.begin(), "full"), std::bad_alloc);
    REQUIRE(v == expected);
}

TEST_CASE("insert with an argument that aliases an element", "[insert][aliasing]")
{
    phreak::fixed_vector<std::string, 8> v{"a", "b", "c"};
    v.insert(v.begin(), v[1]);
    const phreak::fixed_vector<std::string, 8> expected{"b", "a", "b", "c"};
    REQUIRE(v == expected);

    v.insert(v.begin() + 2, v.back());
    const phreak::fixed_vector<std::string, 8> expected2{"b", "a", "c", "b", "c"};
    REQUIRE(v == expected2);
}

TEST_CASE("erase", "[erase]")
{
    SVec v{"1", "2", "3", "4", "5"};

    auto it = v.erase(v.begin() + 1);
    REQUIRE(*it == "3");
    const SVec afterOne{"1", "3", "4", "5"};
    REQUIRE(v == afterOne);

    it = v.erase(v.begin() + 1, v.begin() + 3);
    REQUIRE(*it == "5");
    const SVec afterRange{"1", "5"};
    REQUIRE(v == afterRange);

    it = v.erase(v.begin(), v.begin());
    REQUIRE(it == v.begin());
    REQUIRE(v.size() == 2);

    it = v.erase(v.end() - 1);
    REQUIRE(it == v.end());
    REQUIRE(v.size() == 1);

    it = v.erase(v.begin(), v.end());
    REQUIRE(it == v.end());
    REQUIRE(v.empty());

    IVec trivial{1, 2, 3, 4};
    trivial.erase(trivial.begin(), trivial.begin() + 2);
    const IVec expectTrivial{3, 4};
    REQUIRE(trivial == expectTrivial);
}

TEST_CASE("swap: all call styles and size combinations", "[swap]")
{
    const SVec three{"1", "2", "3"};
    const SVec five{"a", "b", "c", "d", "e"};
    const SVec none{};

    {
        SVec a{three};
        SVec b{five};
        a.swap(b);
        REQUIRE(a == five);
        REQUIRE(b == three);
        a.swap(b);
        REQUIRE(a == three);
        REQUIRE(b == five);
    }
    {
        SVec a{three};
        SVec b{three};
        b[0] = "x";
        const SVec bBefore{b};
        a.swap(b);
        REQUIRE(a == bBefore);
        REQUIRE(b == three);
    }
    {
        SVec a{};
        SVec b{five};
        a.swap(b);
        REQUIRE(a == five);
        REQUIRE(b.empty());
    }
    {
        SVec a{};
        SVec b{};
        a.swap(b);
        REQUIRE(a.empty());
        REQUIRE(b.empty());
    }
    {
        SVec a{three};
        a.swap(a);
        REQUIRE(a == three);
    }
    {
        SVec a{three};
        SVec b{five};
        using std::swap;
        swap(a, b);
        REQUIRE(a == five);
        REQUIRE(b == three);
    }
    {
        SVec a{three};
        SVec b{five};
        std::swap(a, b);
        REQUIRE(a == five);
        REQUIRE(b == three);
    }
}

TEST_CASE("comparison operators", "[compare]")
{
    const IVec a{1, 2, 3};
    const IVec same{1, 2, 3};
    const IVec prefix{1, 2};
    const IVec bigger{1, 2, 4};
    const IVec empty{};

    REQUIRE(a == same);
    REQUIRE_FALSE(a != same);
    REQUIRE_FALSE(a < same);
    REQUIRE_FALSE(a > same);
    REQUIRE(a <= same);
    REQUIRE(a >= same);

    REQUIRE(prefix != a);
    REQUIRE(prefix < a);
    REQUIRE(prefix <= a);
    REQUIRE_FALSE(prefix > a);
    REQUIRE_FALSE(prefix >= a);
    REQUIRE(a > prefix);
    REQUIRE(a >= prefix);
    REQUIRE_FALSE(a < prefix);
    REQUIRE_FALSE(a <= prefix);

    REQUIRE(a < bigger);
    REQUIRE(a <= bigger);
    REQUIRE(bigger > a);
    REQUIRE(bigger >= a);
    REQUIRE_FALSE(bigger < a);

    REQUIRE(empty < a);
    REQUIRE(empty == IVec{});
    REQUIRE_FALSE(empty < IVec{});
}

TEST_CASE("move-only element type", "[types]")
{
    phreak::fixed_vector<std::unique_ptr<int>, 4> v;
    v.emplace_back(std::make_unique<int>(2));
    v.push_back(std::make_unique<int>(3));
    v.insert(v.begin(), std::make_unique<int>(1));
    REQUIRE(v.size() == 3);
    REQUIRE(*v[0] == 1);
    REQUIRE(*v[2] == 3);

    v.erase(v.begin() + 1);
    REQUIRE(*v[1] == 3);

    phreak::fixed_vector<std::unique_ptr<int>, 4> w;
    w.emplace_back(std::make_unique<int>(9));
    v.swap(w);
    REQUIRE(v.size() == 1);
    REQUIRE(*v[0] == 9);
    REQUIRE(w.size() == 2);
}

TEST_CASE("element type without default constructor", "[types]")
{
    phreak::fixed_vector<NoDefault, 4> v;
    v.emplace_back(2);
    v.emplace(v.begin(), 1);
    v.insert(v.end(), NoDefault{3});
    REQUIRE(v.size() == 3);
    REQUIRE(v[0].value == 1);
    REQUIRE(v[2].value == 3);

    v.resize(4, 4);
    REQUIRE(v.back().value == 4);

    v.erase(v.begin());
    REQUIRE(v.front().value == 2);
}

TEST_CASE("over-aligned element type", "[types]")
{
    phreak::fixed_vector<OverAligned, 3> v;
    v.push_back(OverAligned{1});
    v.push_back(OverAligned{2});
    for (const auto& e : v) {
        REQUIRE(reinterpret_cast<std::uintptr_t>(&e) % alignof(OverAligned) == 0);
    }
}

TEST_CASE("every constructed element is destroyed exactly once", "[lifetime]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = -1;
    {
        TVec v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace(v.begin(), 0);
        v.resize(6, 9);
        v.erase(v.begin() + 1, v.begin() + 3);
        v.pop_back();
        v.assign(5, 7);
        v.assign(2, 3);

        TVec copy{v};
        TVec moved{std::move(copy)};
        copy = moved;
        moved = std::move(copy);

        TVec other(7, 5);
        v.swap(other);
        v.swap(other);
        v.resize(1, 0);

        REQUIRE(Tracked::alive == static_cast<int>(v.size() + copy.size() + moved.size() + other.size()));
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("count constructor does not leak when an element throws", "[exception]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = 2;
    REQUIRE_THROWS_AS(TVec(5, 1), std::runtime_error);
    REQUIRE(Tracked::alive == 0);
    Tracked::throwAfter = -1;
}

TEST_CASE("range constructor does not leak when an element throws", "[exception]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = -1;
    {
        std::vector<Tracked> src;
        src.reserve(5);
        for (int i = 0; i < 5; ++i) {
            src.emplace_back(i);
        }
        const int baseline = Tracked::alive;

        Tracked::throwAfter = 2;
        REQUIRE_THROWS_AS(TVec(src.begin(), src.end()), std::runtime_error);
        REQUIRE(Tracked::alive == baseline);
        Tracked::throwAfter = -1;
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("copy constructor does not leak when an element throws", "[exception]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = -1;
    {
        TVec src(4, 1);
        Tracked::throwAfter = 2;
        REQUIRE_THROWS_AS(TVec{src}, std::runtime_error);
        REQUIRE(Tracked::alive == 4);
        Tracked::throwAfter = -1;
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("push_back that throws leaves the vector unchanged", "[exception]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = -1;
    {
        TVec v;
        v.emplace_back(1);
        v.emplace_back(2);
        const Tracked extra{3};

        Tracked::throwAfter = 0;
        REQUIRE_THROWS_AS(v.push_back(extra), std::runtime_error);
        REQUIRE(v.size() == 2);
        REQUIRE(v.back().value == 2);
        REQUIRE(Tracked::alive == 3);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("emplace that throws during construction leaves the vector unchanged", "[exception]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = -1;
    {
        TVec v;
        v.emplace_back(1);
        v.emplace_back(2);

        Tracked::throwAfter = 0;
        REQUIRE_THROWS_AS(v.emplace(v.begin(), 0), std::runtime_error);
        REQUIRE(v.size() == 2);
        REQUIRE(v.front().value == 1);
        REQUIRE(Tracked::alive == 2);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("assign that throws keeps size and live objects consistent", "[exception]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = -1;
    {
        TVec v(3, 0);
        std::vector<Tracked> src;
        src.reserve(6);
        for (int i = 0; i < 6; ++i) {
            src.emplace_back(i);
        }

        Tracked::throwAfter = 1;
        REQUIRE_THROWS_AS(v.assign(src.begin(), src.end()), std::runtime_error);
        REQUIRE(v.size() == 4);
        REQUIRE(Tracked::alive == static_cast<int>(v.size() + src.size()));
        Tracked::throwAfter = -1;
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("swap with a throwing move constructor neither leaks nor double-destroys", "[exception][swap]")
{
    Tracked::alive      = 0;
    Tracked::throwAfter = -1;
    {
        TVec a;
        TVec b;
        for (int i = 0; i < 4; ++i) {
            a.emplace_back(i);
        }
        b.emplace_back(42);

        Tracked::throwAfter = 1;
        REQUIRE_THROWS_AS(a.swap(b), std::runtime_error);
        REQUIRE(Tracked::alive == static_cast<int>(a.size() + b.size()));
        Tracked::throwAfter = -1;
    }
    REQUIRE(Tracked::alive == 0);
}

}
