/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  Last modified:
 *     $Date: 2009-01-20 23:44:27 +0100 (ti, 20 jan 2009) $ by $Author: schulte $
 *     $Revision: 8082 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

namespace Gecode { namespace Int {

  /*
   * Test sharing between Boolean and negated Boolean views
   *
   */
  forceinline BoolTest
  bool_test(const BoolView& b0, const BoolView& b1) {
    return same(b0,b1) ? BT_SAME : BT_NONE;
  }
  forceinline BoolTest
  bool_test(const BoolView& b0, const NegBoolView& b1) {
    return same(b0,b1.base()) ? BT_COMP : BT_NONE;
  }
  forceinline BoolTest
  bool_test(const NegBoolView& b0, const BoolView& b1) {
    return same(b0.base(),b1) ? BT_COMP : BT_NONE;
  }
  forceinline BoolTest
  bool_test(const NegBoolView& b0, const NegBoolView& b1) {
    return same(b0,b1) ? BT_SAME : BT_NONE;
  }

}}

// STATISTICS: int-var

