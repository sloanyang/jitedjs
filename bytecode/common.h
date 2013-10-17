#ifndef COMMON_H
#define COMMON_H

struct VarID {
    VarID(int id): m_id(id){}
    int getInt() const { return m_id; }
    static VarID fromLocal(int localIdx){ ASSERT(localIdx < 128); return VarID(localIdx);}
    static VarID fromConst(int constIdx){ ASSERT(constIdx < 128); return VarID(128 + constIdx);}
    bool isLocal() const { return (m_id >> 7) == 0; }
    bool isConst() const { return (m_id >> 7) == 1; }
    int getLocal() const { ASSERT(isLocal()); return m_id & 0x3f;}
    int getConst() const { ASSERT(isConst()); return m_id & 0x3f; }
    string toString(FuncMeta *meta) const {
        if (m_id >> 7) {
            auto v = meta->constTable[m_id & 0x3f];
            if (v.type == JSVT_String) return format("'%s'", v.data.str->buf);
            else return v.toString();
        }
        else return format("l_%d", m_id & 0x3f);
    }
    JSValue* toValue(JSValue* localConstPtr[2]) const{ return localConstPtr[m_id >> 7] + (m_id & 0x3f);}
private:
    const int m_id;
};

static const int BIT_W_VAR_ID = 8;
static const int BIT_W_META_IDX = 8;
static const int BIT_W_IP = 16;


#endif
