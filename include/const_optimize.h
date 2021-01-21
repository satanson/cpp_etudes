// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/18.
//

#ifndef CPP_ETUDES_INCLUDE_CONST_OPTIMIZE_H_
#define CPP_ETUDES_INCLUDE_CONST_OPTIMIZE_H_

struct ConstState {
  bool a_is_const;
  bool b_is_const;
  bool c_is_const;
  bool d_is_const;
};

template<bool a_is_const, bool b_is_const, bool c_is_const, bool d_is_const>
void const_optimized(){

}
#endif //CPP_ETUDES_INCLUDE_CONST_OPTIMIZE_H_
