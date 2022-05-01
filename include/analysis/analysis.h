#pragma
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "guard.hh"

class Expr;
using ExprPtr = std::shared_ptr<Expr>;
using Exprs = std::vector<ExprPtr>;

class Expr {
public:
    Expr() = default;
    size_t id() { return _id; }
    void set_id(size_t id) { _id = id; }
    virtual ~Expr() {}
    virtual std::string to_string() = 0;

private:
    size_t _id;
};

class SimpleExpr : public Expr {
public:
    SimpleExpr() : Expr() {}
    virtual std::string to_string() = 0;
};

class CompoundExpr : public Expr {
public:
    CompoundExpr(Exprs const& expr) : _children(expr) {}
    virtual std::string to_string() = 0;
    size_t num_children() { return _children.size(); }
    ExprPtr child(size_t i) { return _children[i]; }

private:
    Exprs _children;
};

enum SimpleType {
    TYPE_UNDEFINED = 0,
    TYPE_BOOLEAN,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_UNKNOWN
};
struct Undefined {};
struct Unknown {};
template <SimpleType T>
struct DataTypeTrait {};
#define DEFINE_DATATYPE_TRAIT(t, dt)       \
    template <>                            \
    struct DataTypeTrait<t> {              \
        using DataType = dt;               \
        static constexpr char* Name = #dt; \
    };

DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_BOOLEAN, bool);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_INT8, int8_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_INT16, int16_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_INT32, int32_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_INT64, int64_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_UINT8, uint8_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_UINT16, uint16_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_UINT32, uint32_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_UINT64, uint64_t);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_FLOAT, float);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_DOUBLE, double);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_STRING, std::string);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_UNDEFINED, Undefined);
DEFINE_DATATYPE_TRAIT(SimpleType::TYPE_UNKNOWN, Unknown);

VALUE_GUARD(SimpleType, UndefinedTypeGuard, type_is_undefined, TYPE_UNDEFINED)
VALUE_GUARD(SimpleType, UnknownTypeGuard, type_is_known, TYPE_UNKNOWN)
VALUE_GUARD(SimpleType, BooleanTypeGuard, type_is_boolean, TYPE_BOOLEAN)
VALUE_GUARD(SimpleType, SignedIntegerTypeGuard, type_is_signed_integer, TYPE_INT8, TYPE_INT16, TYPE_INT32, TYPE_INT64)
VALUE_GUARD(SimpleType, UnsignedIntegerTypeGuard, type_is_unsigned_integer, TYPE_UINT8, TYPE_UINT16, TYPE_UINT32,
            TYPE_UINT64)
VALUE_GUARD(SimpleType, FloatPointingTypeGuard, type_is_float_pointing, TYPE_FLOAT, TYPE_DOUBLE)
VALUE_GUARD(SimpleType, StringTypeGuard, type_is_string, TYPE_STRING)
UNION_VALUE_GUARD(SimpleType, IntegerTypeGuard, type_is_integer, type_is_signed_integer_struct,
                  type_is_unsigned_integer_struct)
UNION_VALUE_GUARD(SimpleType, NumberTypeGuard, type_is_number, type_is_integer_struct, type_is_float_pointing_struct)
UNION_VALUE_GUARD(SimpleType, FixedLengthTypeGuard, type_is_fixed_length, type_is_number_struct, type_is_boolean_struct)
UNION_VALUE_GUARD(SimpleType, DefinedTypeGuard, type_is_defined, type_is_fixed_length_struct, type_is_string_struct)

template <SimpleType T>
using DataType = typename DataTypeTrait<T>::DataType;
class LiteralExprBase : public SimpleExpr {};
template <SimpleType T>
class LiteralExpr : LiteralExprBase {
public:
    using Type = DataType<T>;
    LiteralExpr(const Type& value) : _value(value) {}
    Type get_value() { return _value; }

private:
    Type _value;
};

class VarDefExpr : SimpleExpr {
public:
    VarDefExpr(std::string const& var, ExprPtr const& expr) : _var(var), _expr(expr) {}

public:
    const std::string& get_var() const { return _var; }
    const std::shared_ptr<Expr>& get_expr() const { return _expr; }

private:
    std::string _var;
    ExprPtr _expr;
};

class VarUseExpr : SimpleExpr {
public:
    std::string get_var() { return _var; }

private:
    std::string _var;
};
enum OpType { OP_ADD, OP_SUB, OP_MUL, OP_DIV };
class ArithmeticExpr : CompoundExpr {
public:
    ArithmeticExpr(OpType op, Exprs const& exprs) : CompoundExpr(exprs), _op(op) {}
    OpType op() { return _op; }

private:
    OpType _op;
};

class PhiExpr : CompoundExpr {
public:
    PhiExpr(Exprs const& exprs) : CompoundExpr(exprs) {}
};

class Statement;
using StatmentPtr = std::shared_ptr<Statement>;
using Statements = std::vector<StatmentPtr>;
class Statement {
public:
    virtual ~Statement(){};
    void set_id(size_t id) { _id = id; }
    size_t id() { return _id; }
    virtual std::string to_string() = 0;

private:
    size_t _id;
};

class SimpleStmt : public Statement {
public:
    SimpleStmt(ExprPtr const& var_def) : _var_def(var_def) {}

private:
    ExprPtr _var_def;
};

class CompoundStmt : public Statement {
public:
};

class BranchStmt : public CompoundStmt {
public:
    BranchStmt(size_t dest_id, std::optional<size_t> const& else_dest_id, std::optional<ExprPtr> const& cond_expr)
            : _dest_id(dest_id), _else_dest_id(else_dest_id), _cond_expr(cond_expr) {}
    size_t dest_id() { return _dest_id; }
    std::optional<size_t> else_dest_id() { return _else_dest_id; }
    std::optional<ExprPtr> cond_expr() { return _cond_expr; }

private:
    size_t _dest_id;
    std::optional<size_t> _else_dest_id;
    std::optional<ExprPtr> _cond_expr;
};
class StmtBlock;
using StmtBlockPtr = std::shared_ptr<StmtBlock>;
using StmtBlocks = std::vector<StmtBlockPtr>;
using Id2StmtBlocks = std::map<size_t, StmtBlockPtr>;
class StmtBlock {
public:
    explicit StmtBlock(Statements const& stmts) : _stmts(stmts) {}
    size_t num_stmts() { return _stmts.size(); }
    const StatmentPtr& enter_stmt() { return _stmts.front(); }
    const StatmentPtr& leave_stmt() { return _stmts.back(); }
    std::vector<size_t>& succ_ids() { return _succ_ids; }
    std::vector<size_t>& pred_ids() { return _pred_ids; }
    void set_succ_ids(std::vector<size_t> const& succ_ids) { _succ_ids = succ_ids; }
    void set_pred_ids(std::vector<size_t> const& pred_ids) { _succ_ids = pred_ids; }

private:
    std::vector<size_t> _succ_ids;
    std::vector<size_t> _pred_ids;
    Statements _stmts;
};

class Cfg {
public:
    StmtBlockPtr& begin_block() { return _begin_block; }
    StmtBlocks& end_blocks() { return _end_blocks; }
    StmtBlocks blocks() {
        StmtBlocks blocks;
        for (auto it = _blocks.begin(); it != _blocks.end(); ++it) {
            blocks.emplace_back(it->second);
        }
        return blocks;
    }
    StmtBlocks pred(StmtBlockPtr const& block) {
        StmtBlocks  pred_blocks;
        for (auto id:block->pred_ids()){
            pred_blocks.push_back(_blocks[id]);
        }
        return pred_blocks;
    }
    StmtBlocks succ(StmtBlockPtr const& block) {
        StmtBlocks  succ_blocks;
        for (auto id:block->succ_ids()){
            succ_blocks.push_back(_blocks[id]);
        }
        return succ_blocks;
    }
private:
    StmtBlockPtr _begin_block;
    StmtBlocks _end_blocks;
    Id2StmtBlocks _blocks;
};