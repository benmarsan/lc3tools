#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

namespace core {
    typedef enum {
          OPER_TYPE_FIXED = 2
        , OPER_TYPE_NUM
        , OPER_TYPE_LABEL
        , OPER_TYPE_REG
    } OperType;

    class Operand
    {
    public:
        OperType type;

        // used by assembler
        std::string type_str;
        uint32_t width;

        // used by simulator
        uint32_t value;

        Operand(OperType type, std::string const & type_str, uint32_t width);
        virtual ~Operand(void) = default;

        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) = 0;

        bool isEqualType(OperType other) const;
    };

    class Instruction
    {
    public:
        std::string name;
        std::vector<Operand *> operands;

        Instruction(std::string const & name, std::vector<Operand *> const & operands);
        virtual ~Instruction(void);

        uint32_t getNumOperands(void) const;
        virtual void execute(MachineState & state) = 0;
        virtual Instruction * clone(void) const = 0;
    };

    class InstructionHandler
    {
    public:
        InstructionHandler(void);
        virtual ~InstructionHandler(void);

        std::map<std::string, uint32_t> const & getRegs(void) { return regs; }
    protected:
        std::vector<Instruction *> instructions;
        std::map<std::string, uint32_t> regs;
    };

    class FixedOperand : public Operand
    {
    public:
        FixedOperand(uint32_t width, uint32_t value) : Operand(OPER_TYPE_FIXED, "fixed", width) { this->value = value; }
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class RegOperand : public Operand
    {
    public:
        RegOperand(uint32_t width) : Operand(OPER_TYPE_REG, "reg", width) {}
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class NumOperand : public Operand
    {
    public:
        bool sext;

        NumOperand(uint32_t width, bool sext) : Operand(OPER_TYPE_NUM, "imm", width) { this->sext = sext; }
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class LabelOperand : public Operand
    {
    public:
        LabelOperand(uint32_t width) : Operand(OPER_TYPE_LABEL, "label", width) {}
        virtual uint32_t encode(bool log_enable, AssemblerLogger const & logger, std::string const & filename,
            std::string const & line, Token const * inst, Token const * operand, uint32_t oper_count,
            std::map<std::string, uint32_t> const & registers, std::map<std::string, uint32_t> const & labels) override;
    };

    class ADDRInstruction : public Instruction
    {
    public:
        ADDRInstruction(void) : Instruction("add", {
            new FixedOperand(4, 0x1),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(3, 0x0),
            new RegOperand(3)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual ADDRInstruction * clone(void) const override { return new ADDRInstruction(*this); }
    };

    class ADDIInstruction : public Instruction
    {
    public:
        ADDIInstruction(void) : Instruction("add", {
            new FixedOperand(4, 0x1),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(1, 0x1),
            new NumOperand(5, true)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual ADDIInstruction * clone(void) const override { return new ADDIInstruction(*this); }
    };

    class ANDRInstruction : public Instruction
    {
    public:
        ANDRInstruction(void) : Instruction("and", {
            new FixedOperand(4, 0x5),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(3, 0x0),
            new RegOperand(3)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual ANDRInstruction * clone(void) const override { return new ANDRInstruction(*this); }
    };

    class ANDIInstruction : public Instruction
    {
    public:
        ANDIInstruction(void) : Instruction("and", {
            new FixedOperand(4, 0x5),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(1, 0x1),
            new NumOperand(5, true)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual ANDIInstruction * clone(void) const override { return new ANDIInstruction(*this); }
    };

    class JMPInstruction : public Instruction
    {
    public:
        using Instruction::Instruction;
        JMPInstruction(void) : Instruction("jmp", {
            new FixedOperand(4, 0xc),
            new FixedOperand(3, 0x0),
            new RegOperand(3),
            new FixedOperand(6, 0x0)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual JMPInstruction * clone(void) const override { return new JMPInstruction(*this); }
    };

    class JSRInstruction : public Instruction
    {
    public:
        using Instruction::Instruction;
        JSRInstruction(void) : Instruction("jsr", {
            new FixedOperand(4, 0x4),
            new FixedOperand(1, 0x1),
            new LabelOperand(11)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual JSRInstruction * clone(void) const override { return new JSRInstruction(*this); }
    };

    class JSRRInstruction : public Instruction
    {
    public:
        JSRRInstruction(void) : Instruction("jsrr", {
            new FixedOperand(4, 0x4),
            new FixedOperand(1, 0x0),
            new FixedOperand(2, 0x0),
            new RegOperand(3),
            new FixedOperand(6, 0x0)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual JSRRInstruction * clone(void) const override { return new JSRRInstruction(*this); }
    };

    class LDInstruction : public Instruction
    {
    public:
        LDInstruction(void) : Instruction("ld", {
            new FixedOperand(4, 0x2),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual LDInstruction * clone(void) const override { return new LDInstruction(*this); }
    };

    class LDIInstruction : public Instruction
    {
    public:
        LDIInstruction(void) : Instruction("ldi", {
            new FixedOperand(4, 0xa),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual LDIInstruction * clone(void) const override { return new LDIInstruction(*this); }
    };

    class LDRInstruction : public Instruction
    {
    public:
        LDRInstruction(void) : Instruction("ldr", {
            new FixedOperand(4, 0x6),
            new RegOperand(3),
            new RegOperand(3),
            new LabelOperand(6)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual LDRInstruction * clone(void) const override { return new LDRInstruction(*this); }
    };

    class LEAInstruction : public Instruction
    {
    public:
        LEAInstruction(void) : Instruction("lea", {
            new FixedOperand(4, 0xe),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual LEAInstruction * clone(void) const override { return new LEAInstruction(*this); }
    };

    class NOTInstruction : public Instruction
    {
    public:
        NOTInstruction(void) : Instruction("not", {
            new FixedOperand(4, 0x0),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(6, 0x3f)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual NOTInstruction * clone(void) const override { return new NOTInstruction(*this); }
    };

    class RETInstruction : public JMPInstruction
    {
    public:
        RETInstruction(void) : JMPInstruction("ret", {
            new FixedOperand(4, 0xc),
            new FixedOperand(3, 0x0),
            new FixedOperand(3, 0x7),
            new FixedOperand(6, 0x0)
        }) {}
        virtual RETInstruction * clone(void) const override { return new RETInstruction(*this); }
    };

    class RTIInstruction : public Instruction
    {
    public:
        RTIInstruction(void) : Instruction("rti", {
            new FixedOperand(4, 0x0),
            new RegOperand(3),
            new RegOperand(3),
            new FixedOperand(6, 0x3f)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual RTIInstruction * clone(void) const override { return new RTIInstruction(*this); }
    };

    class STInstruction : public Instruction
    {
    public:
        STInstruction(void) : Instruction("st", {
            new FixedOperand(4, 0x3),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual STInstruction * clone(void) const override { return new STInstruction(*this); }
    };

    class STIInstruction : public Instruction
    {
    public:
        STIInstruction(void) : Instruction("sti", {
            new FixedOperand(4, 0xb),
            new RegOperand(3),
            new LabelOperand(9)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual STIInstruction * clone(void) const override { return new STIInstruction(*this); }
    };

    class STRInstruction : public Instruction
    {
    public:
        STRInstruction(void) : Instruction("str", {
            new FixedOperand(4, 0x7),
            new RegOperand(3),
            new RegOperand(3),
            new LabelOperand(6)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual STRInstruction * clone(void) const override { return new STRInstruction(*this); }
    };

    class TRAPInstruction : public Instruction
    {
    public:
        TRAPInstruction(void) : Instruction("trap", {
            new FixedOperand(4, 0xf),
            new FixedOperand(4, 0x0),
            new NumOperand(8, false)
        }) {}
        virtual void execute(MachineState & state) override;
        virtual TRAPInstruction * clone(void) const override { return new TRAPInstruction(*this); }
    };
};

#endif