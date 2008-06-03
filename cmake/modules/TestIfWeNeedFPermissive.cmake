include (CheckCXXSourceCompiles)

macro (TEST_IF_WE_NEED_FPERMISSIVE)
  SET(_TEST_IF_WE_NEED_FPERMISSIVE_SOURCE_CODE
#This checks if we need -fpermissive to work around a problem with Boost/Python and GCC >= GCC 4.3.1
#See: http://gcc.gnu.org/gcc-4.3/porting_to.html
#GCC by default no longer accepts code such as 
"
template <class _Tp> class auto_ptr {};
template <class _Tp>
struct counted_ptr
{
  auto_ptr<_Tp> auto_ptr();
};
int main() {return 0;}
"
#
#
#
  )
	
  CHECK_CXX_SOURCE_COMPILES("${_TEST_IF_WE_NEED_FPERMISSIVE_SOURCE_CODE}" COMPILES_WITHOUT_FPERMISSIVE)

  set (NEED_FPERMISSIVE NOT COMPILES_WITHOUT_FPERMISSIVE)

endmacro (TEST_IF_WE_NEED_FPERMISSIVE)
#but will issue the diagnostic
#
#error: declaration of 'auto_ptr<_Tp> counted_ptr<_Tp>::auto_ptr()'
#error: changes meaning of 'auto_ptr' from 'class auto_ptr<_Tp>'
#
#The reference to struct auto_ptr needs to be qualified here, or the name of the member function changed to be unambiguous.
#
#template <class _Tp> class auto_ptr {};
#template <class _Tp>
#struct counted_ptr
#{
#  ::auto_ptr<_Tp> auto_ptr();
#};
#
#In addition, -fpermissive can be used as a temporary workaround to convert the error into a warning until the code is fixed. 
#Note that then in some case name lookup will not be standard conforming. 