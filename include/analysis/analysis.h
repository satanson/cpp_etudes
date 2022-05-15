#pragma
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
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

template <typename E>
class ExprFactory {
public:
    template <typename... Args>
    static ExprPtr create(Args&&... args) {
        return std::dynamic_pointer_cast<Expr>(std::make_shared<E>(std::forward<Args>(args)...));
    }
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
class LiteralExpr : public LiteralExprBase {
public:
    using Type = DataType<T>;
    LiteralExpr(const Type& value) : _value(value) {}
    Type get_value() { return _value; }
    std::string to_string() override {
        constexpr auto* name = DataTypeTrait<T>::Name;
        std::stringstream ss;
        ss << "LiteralExpr(type=" << name << ", value=" << _value << ")";
        return ss.str();
    }

private:
    Type _value;
};

class VarDefExpr : public SimpleExpr {
public:
    VarDefExpr(std::string const& var, ExprPtr const& expr) : _var(var), _expr(expr) {}

public:
    const std::string& get_var() const { return _var; }
    const std::shared_ptr<Expr>& get_expr() const { return _expr; }
    std::string to_string() override {
        std::stringstream ss;
        ss << "VarDefExpr(var=" << _var << ", expr=" << _expr->to_string() << ")";
        return ss.str();
    }

private:
    std::string _var;
    ExprPtr _expr;
};

class VarUseExpr : public SimpleExpr {
public:
    VarUseExpr(const std::string& var) : _var(var) {}
    std::string get_var() { return _var; }
    std::string to_string() override {
        std::stringstream ss;
        ss << "VarUseExpr(var=" << _var << ")";
        return ss.str();
    }

private:
    std::string _var;
};
enum OpType { OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_EQ, OP_NE, OP_LE, OP_GE, OP_LT, OP_GT };
class ArithmeticExpr : public CompoundExpr {
public:
    ArithmeticExpr(OpType op, Exprs const& exprs) : CompoundExpr(exprs), _op(op) {}
    OpType op() { return _op; }
    std::string to_string() override {
        std::stringstream ss;
        std::string op_name = "unknown";
        switch (_op) {
        case OP_ADD:
            op_name = "add(+)";
            break;
        case OP_SUB:
            op_name = "sub(-)";
            break;
        case OP_MUL:
            op_name = "mul(*)";
            break;
        case OP_DIV:
            op_name = "div(/)";
            break;
        case OP_MOD:
            op_name = "mod(%)";
            break;
        }
        ss << "ArithmeticExpr(op=" << op_name << ", lhs=" << child(0)->to_string() << ", rhs=" << child(1)->to_string()
           << ")";
        return ss.str();
    }

private:
    OpType _op;
};

class RelationExpr : public CompoundExpr {
public:
    RelationExpr(OpType op, Exprs const& exprs) : CompoundExpr(exprs), _op(op) {}
    OpType op() { return _op; }
    std::string to_string() override {
        std::stringstream ss;
        std::string op_name = "unknown";
        switch (_op) {
        case OP_EQ:
            op_name = "eq(==)";
            break;
        case OP_NE:
            op_name = "ne(!=)";
            break;
        case OP_GE:
            op_name = "ge(>=)";
            break;
        case OP_LE:
            op_name = "le(<=)";
            break;
        case OP_GT:
            op_name = "gt(>)";
            break;
        case OP_LT:
            op_name = "lt(<)";
            break;
        }
        ss << "RelationExpr(op=" << op_name << ", lhs=" << child(0)->to_string() << ", rhs=" << child(1)->to_string()
           << ")";
        return ss.str();
    }

private:
    OpType _op;
};

class PhiExpr : CompoundExpr {
public:
    PhiExpr(Exprs const& exprs) : CompoundExpr(exprs) {}
    std::string to_string() override {
        std::stringstream ss;
        ss << "PhiExpr(";
        if (num_children() >= 1) {
            ss << "expr0=" << child(0)->to_string();
        }
        for (int i = 1; i < num_children(); ++i) {
            ss << ", expr" << i << "=" << child(i)->to_string();
        }
        ss << ")";
        return ss.str();
    }
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
    explicit SimpleStmt(ExprPtr const& var_def) : _var_def(var_def) {}
    std::string to_string() override {
        std::stringstream ss;
        ss << "SimpleStmt(var_def=" << _var_def->to_string() << ")";
        return ss.str();
    }

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
    std::string to_string() {
        std::stringstream ss;
        ss << "BranchStmt(cond_expr=" << (_cond_expr.has_value() ? "none" : _cond_expr.value()->to_string())
           << ", dest_id=" << _dest_id << ", else_dest_id" << (_else_dest_id.has_value() ? -1 : _else_dest_id.value())
           << ")";
        return ss.str();
    }

private:
    size_t _dest_id;
    std::optional<size_t> _else_dest_id;
    std::optional<ExprPtr> _cond_expr;
};

template <typename S>
struct StmtFactory {
    template <typename... Args>
    static StatmentPtr create(Args&&... args) {
        return std::dynamic_pointer_cast<Statement>(std::make_shared<S>(std::forward<Args>(args)...));
    }
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
    void set_id(size_t id) { _id = id; }
    size_t id() const { return _id; }

private:
    size_t _id;
    std::vector<size_t> _succ_ids;
    std::vector<size_t> _pred_ids;
    Statements _stmts;
};

class Cfg {
public:
    StmtBlockPtr& begin_block() { return _blocks[_begin_block_id]; }
    StmtBlocks end_blocks() {
        std::vector<StmtBlockPtr> blk;
        blk.reserve(_end_block_ids.size());
        for (auto id : _end_block_ids) {
            blk.push_back(_blocks[id]);
        }
        return blk;
    }
    void add(StmtBlockPtr&& blk) {
        if (blk->succ_ids().empty()) {
            _begin_block_id = blk->id();
        }
        if (blk->pred_ids().empty()) {
            _end_block_ids.push_back(blk->id());
        }
        _blocks.emplace(blk->id(), std::move(blk));
    }
    StmtBlocks blocks() {
        StmtBlocks blocks;
        for (auto it = _blocks.begin(); it != _blocks.end(); ++it) {
            blocks.emplace_back(it->second);
        }
        return blocks;
    }
    StmtBlocks pred(StmtBlockPtr const& block) {
        StmtBlocks pred_blocks;
        for (auto id : block->pred_ids()) {
            pred_blocks.push_back(_blocks[id]);
        }
        return pred_blocks;
    }
    StmtBlocks succ(StmtBlockPtr const& block) {
        StmtBlocks succ_blocks;
        for (auto id : block->succ_ids()) {
            succ_blocks.push_back(_blocks[id]);
        }
        return succ_blocks;
    }

private:
    size_t _begin_block_id;
    std::vector<size_t> _end_block_ids;
    Id2StmtBlocks _blocks;
};