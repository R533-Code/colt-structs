#ifndef HG_COLT_EXPECTED
#define HG_COLT_EXPECTED

namespace colt
{
  template<typename ExpectedT, typename ErrorT>
  class Expected
  {
    union exp_buffer
    {
      ExpectedT expected;
      ErrorT error;
    };

    bool is_error;

  public:

  };
}

#endif //!HG_COLT_EXPECTED