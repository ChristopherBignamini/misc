#include <tuple>
#include <iostream>


/////// Create your "minimal" type list here
using type_tuple = std::tuple<char, short, int, long, long long>;
///////


/////// Recursive minimal type identification
template <unsigned int Size, short TypeIndex>
struct magic_helper;

// TODO: reverse recursion direction
template <unsigned int Size, short TypeIndex>
struct magic_helper : magic_helper<Size,TypeIndex+1>
{

  static constexpr short type_index = 
    (sizeof(typename std::tuple_element<TypeIndex, type_tuple>::type)/Size) ? TypeIndex : magic_helper<Size,TypeIndex+1>::type_index;
  
};

template<unsigned int Size>
struct magic_helper<Size, std::tuple_size<type_tuple>::value - 1 >{
  static constexpr short type_index = std::tuple_size<type_tuple>::value - 1;
};
///////

/////// Minimal tye identification user interface
template <typename Type>
struct magic {

  using helper = magic_helper<sizeof(Type),0>;  
  using type = typename std::tuple_element<helper::type_index, type_tuple>::type;

  // TODO: implement data move/copy methods
};
///////

int main(){

  std::cout<<magic<char>::helper::type_index<<std::endl;
  std::cout<<magic<short>::helper::type_index<<std::endl;
  std::cout<<magic<int>::helper::type_index<<std::endl;
  std::cout<<magic<long>::helper::type_index<<std::endl;
  std::cout<<magic<long long>::helper::type_index<<std::endl;  
  std::cout<<magic<char[2]>::helper::type_index<<std::endl;

  static_assert(std::is_same<magic<char>::type, char>::value,"Error, this should be a char!");
  static_assert(std::is_same<magic<short>::type, short>::value,"Error, this should be a short!");
  static_assert(std::is_same<magic<int>::type, int>::value,"Error, this should be a int!");
  static_assert(std::is_same<magic<long>::type, long>::value,"Error, this should be a long!");
  static_assert(std::is_same<magic<long long>::type, long>::value,"Error, this should be a long!");
  static_assert(std::is_same<magic<char[2]>::type, short>::value,"Error, this should be a short!");

}
