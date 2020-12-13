#include "qaesencryption.h"


QByteArray QAESEncryption::Crypt(QAESEncryption::AES level, QAESEncryption::MODE mode, const QByteArray &rawText,
                                 const QByteArray &key, const QByteArray &iv, QAESEncryption::PADDING padding)
{
    return QAESEncryption(level, mode, padding).encode(rawText, key, iv);
}

QByteArray QAESEncryption::Decrypt(QAESEncryption::AES level, QAESEncryption::MODE mode, const QByteArray &rawText,
                                   const QByteArray &key, const QByteArray &iv, QAESEncryption::PADDING padding)
{
     return QAESEncryption(level, mode, padding).decode(rawText, key, iv);
}

QByteArray QAESEncryption::ExpandKey(QAESEncryption::AES level, QAESEncryption::MODE mode, const QByteArray &key)
{
     return QAESEncryption(level, mode).expandKey(key);
}

QByteArray QAESEncryption::RemovePadding(const QByteArray &rawText, QAESEncryption::PADDING padding)
{
    QByteArray ret(rawText);
    switch (padding)
    {
    case PADDING::ZERO:

        while (ret.at(ret.length()-1) == 0x00)
            ret.remove(ret.length()-1, 1);
        break;
    case PADDING::PKCS7:
        ret.remove(ret.length() - ret.at(ret.length()-1), ret.at(ret.length()-1));
        break;
    case PADDING::ISO:
        ret.truncate(ret.lastIndexOf(0x80));
        break;
    default:
        //do nothing
        break;
    }
    return ret;
}


inline quint8 xTime(quint8 x){
  return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

inline quint8 multiply(quint8 x, quint8 y){
  return (((y & 1) * x) ^ ((y>>1 & 1) * xTime(x)) ^ ((y>>2 & 1) * xTime(xTime(x))) ^ ((y>>3 & 1)
            * xTime(xTime(xTime(x)))) ^ ((y>>4 & 1) * xTime(xTime(xTime(xTime(x))))));
}




QAESEncryption::QAESEncryption(QAESEncryption::AES level, QAESEncryption::MODE mode, PADDING padding)
    : m_nb(4), m_blocklen(16), m_level(level), m_mode(mode), m_padding(padding)
{
    m_state = NULL;

    switch (level)
    {
    case AES_128: {
        AES128 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    case AES_192: {
        AES192 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    case AES_256: {
        AES256 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    default: {
        AES128 aes;
        m_nk = aes.nk;
        m_keyLen = aes.keylen;
        m_nr = aes.nr;
        m_expandedKey = aes.expandedKey;
        }
        break;
    }

}
QByteArray QAESEncryption::getPadding(int currSize, int alignment)
{
    QByteArray ret(0);
    int size = (alignment - currSize % alignment) % alignment;
    if (size == 0) return ret;
    switch(m_padding)
    {
    case PADDING::ZERO:
        ret.insert(0, size, 0x00);
        break;
    case PADDING::PKCS7:
        ret.insert(0, size, size);
        break;
    case PADDING::ISO:
        ret.insert(0, 0x80);
        ret.insert(1, size, 0x00);
        break;
    default:
        ret.insert(0, size, 0x00);
        break;
    }
    return ret;
}

QByteArray QAESEncryption::expandKey(const QByteArray &key)
{
  int i, k;
  quint8 tempa[4]; // Used for the column/row operations
  QByteArray roundKey(key);


  for(i = m_nk; i < m_nb * (m_nr + 1); i++)
  {
    tempa[0] = (quint8) roundKey.at((i-1) * 4 + 0);
    tempa[1] = (quint8) roundKey.at((i-1) * 4 + 1);
    tempa[2] = (quint8) roundKey.at((i-1) * 4 + 2);
    tempa[3] = (quint8) roundKey.at((i-1) * 4 + 3);

    if (i % m_nk == 0)
    {

        k = tempa[0];
        tempa[0] = tempa[1];
        tempa[1] = tempa[2];
        tempa[2] = tempa[3];
        tempa[3] = k;

        // Function Subword()
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);

        tempa[0] =  tempa[0] ^ Rcon[i/m_nk];
    }
    if (m_level == AES_256 && i % m_nk == 4)
    {
        // Function Subword()
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
    }
    roundKey.insert(i * 4 + 0, (quint8) roundKey.at((i - m_nk) * 4 + 0) ^ tempa[0]);
    roundKey.insert(i * 4 + 1, (quint8) roundKey.at((i - m_nk) * 4 + 1) ^ tempa[1]);
    roundKey.insert(i * 4 + 2, (quint8) roundKey.at((i - m_nk) * 4 + 2) ^ tempa[2]);
    roundKey.insert(i * 4 + 3, (quint8) roundKey.at((i - m_nk) * 4 + 3) ^ tempa[3]);
  }
  return roundKey;
}


void QAESEncryption::addRoundKey(const quint8 round, const QByteArray expKey)
{
  QByteArray::iterator it = m_state->begin();
  for(int i=0; i < 16; ++i)
      it[i] = (quint8) it[i] ^ (quint8) expKey.at(round * m_nb * 4 + (i/4) * m_nb + (i%4));
}


void QAESEncryption::subBytes()
{
  QByteArray::iterator it = m_state->begin();
  for(int i = 0; i < 16; i++)
    it[i] = getSBoxValue((quint8) it[i]);
}


void QAESEncryption::shiftRows()
{
    QByteArray::iterator it = m_state->begin();
    quint8 temp;


     //Shift 1 to left
    temp   = (quint8)it[1];
    it[1]  = (quint8)it[5];
    it[5]  = (quint8)it[9];
    it[9]  = (quint8)it[13];
    it[13] = (quint8)temp;

    //Shift 2 to left
    temp   = (quint8)it[2];
    it[2]  = (quint8)it[10];
    it[10] = (quint8)temp;
    temp   = (quint8)it[6];
    it[6]  = (quint8)it[14];
    it[14] = (quint8)temp;

    //Shift 3 to left
    temp   = (quint8)it[3];
    it[3]  = (quint8)it[15];
    it[15] = (quint8)it[11];
    it[11] = (quint8)it[7];
    it[7]  = (quint8)temp;
}


void QAESEncryption::mixColumns()
{
  QByteArray::iterator it = m_state->begin();
  quint8 tmp, tm, t;

  for(int i = 0; i < 16; i += 4){
    t       = (quint8)it[i];
    tmp     =  (quint8)it[i] ^ (quint8)it[i+1] ^ (quint8)it[i+2] ^ (quint8)it[i+3] ;

    tm      = xTime( (quint8)it[i] ^ (quint8)it[i+1] );
    it[i]   = (quint8)it[i] ^ (quint8)tm ^ (quint8)tmp;

    tm      = xTime( (quint8)it[i+1] ^ (quint8)it[i+2]);
    it[i+1] = (quint8)it[i+1] ^ (quint8)tm ^ (quint8)tmp;

    tm      = xTime( (quint8)it[i+2] ^ (quint8)it[i+3]);
    it[i+2] =(quint8)it[i+2] ^ (quint8)tm ^ (quint8)tmp;

    tm      = xTime((quint8)it[i+3] ^ (quint8)t);
    it[i+3] =(quint8)it[i+3] ^ (quint8)tm ^ (quint8)tmp;
  }
}


void QAESEncryption::invMixColumns()
{
  QByteArray::iterator it = m_state->begin();
  quint8 a,b,c,d;
  for(int i = 0; i < 16; i+=4){
    a = (quint8) it[i];
    b = (quint8) it[i+1];
    c = (quint8) it[i+2];
    d = (quint8) it[i+3];

    it[i]   = (quint8) (multiply(a, 0x0e) ^ multiply(b, 0x0b) ^ multiply(c, 0x0d) ^ multiply(d, 0x09));
    it[i+1] = (quint8) (multiply(a, 0x09) ^ multiply(b, 0x0e) ^ multiply(c, 0x0b) ^ multiply(d, 0x0d));
    it[i+2] = (quint8) (multiply(a, 0x0d) ^ multiply(b, 0x09) ^ multiply(c, 0x0e) ^ multiply(d, 0x0b));
    it[i+3] = (quint8) (multiply(a, 0x0b) ^ multiply(b, 0x0d) ^ multiply(c, 0x09) ^ multiply(d, 0x0e));
  }
}


void QAESEncryption::invSubBytes()
{
    QByteArray::iterator it = m_state->begin();
    for(int i = 0; i < 16; ++i)
        it[i] = getSBoxInvert((quint8) it[i]);
}

void QAESEncryption::invShiftRows()
{
    QByteArray::iterator it = m_state->begin();
    uint8_t temp;


    temp   = (quint8)it[13];
    it[13] = (quint8)it[9];
    it[9]  = (quint8)it[5];
    it[5]  = (quint8)it[1];
    it[1]  = (quint8)temp;

    //Shift 2
    temp   = (quint8)it[10];
    it[10] = (quint8)it[2];
    it[2]  = (quint8)temp;
    temp   = (quint8)it[14];
    it[14] = (quint8)it[6];
    it[6]  = (quint8)temp;

    //Shift 3
    temp   = (quint8)it[15];
    it[15] = (quint8)it[3];
    it[3]  = (quint8)it[7];
    it[7]  = (quint8)it[11];
    it[11] = (quint8)temp;
}

QByteArray QAESEncryption::byteXor(const QByteArray &a, const QByteArray &b)
{
  QByteArray::const_iterator it_a = a.begin();
  QByteArray::const_iterator it_b = b.begin();
  QByteArray ret;

  for(int i = 0; i < m_blocklen; i++)
      ret.insert(i,it_a[i] ^ it_b[i]);

  return ret;
}


QByteArray QAESEncryption::cipher(const QByteArray &expKey, const QByteArray &in)
{


  QByteArray output(in);
  m_state = &output;


  addRoundKey(0, expKey);


  for(quint8 round = 1; round < m_nr; ++round){
    subBytes();
    shiftRows();
    mixColumns();
    addRoundKey(round, expKey);
  }

  subBytes();
  shiftRows();
  addRoundKey(m_nr, expKey);

  return output;
}

QByteArray QAESEncryption::invCipher(const QByteArray &expKey, const QByteArray &in)
{
    //m_state is the input buffer.... handle it!
    QByteArray output(in);
    m_state = &output;

    // Add the First round key to the state before starting the rounds.
    addRoundKey(m_nr, expKey);

    // There will be Nr rounds.
    // The first Nr-1 rounds are identical.
    // These Nr-1 rounds are executed in the loop below.
    for(quint8 round=m_nr-1; round>0 ; round--){
        invShiftRows();
        invSubBytes();
        addRoundKey(round, expKey);
        invMixColumns();
    }

    // The last round is given below.
    // The MixColumns function is not here in the last round.
    invShiftRows();
    invSubBytes();
    addRoundKey(0, expKey);

    return output;
}

QByteArray QAESEncryption::encode(const QByteArray &rawText, const QByteArray &key, const QByteArray &iv)
{
    if (m_mode >= CBC && (iv.isNull() || iv.size() != m_blocklen))
       return QByteArray();

    QByteArray ret;
    QByteArray expandedKey = expandKey(key);
    QByteArray alignedText(rawText);
    QByteArray ivTemp(iv);

    //Fill array with padding
    alignedText.append(getPadding(rawText.size(), m_blocklen));

    //Preparation for CFB
    if (m_mode == CFB)
        ret.append(byteXor(alignedText.mid(0, m_blocklen), cipher(expandedKey, iv)));

    //Looping thru all blocks
    for(int i=0; i < alignedText.size(); i+= m_blocklen){
        switch(m_mode)
        {
        case ECB:
            ret.append(cipher(expandedKey, alignedText.mid(i, m_blocklen)));
            break;
        case CBC:
            alignedText.replace(i, m_blocklen, byteXor(alignedText.mid(i, m_blocklen),ivTemp));
            ret.append(cipher(expandedKey, alignedText.mid(i, m_blocklen)));
            ivTemp = ret.mid(i, m_blocklen);
            break;
        case CFB:
            if (i+m_blocklen < alignedText.size())
                ret.append(byteXor(alignedText.mid(i+m_blocklen, m_blocklen),
                                   cipher(expandedKey, ret.mid(i, m_blocklen))));
            break;
        default:
            //do nothing
            break;
        }
    }
    return ret;
}

QByteArray QAESEncryption::decode(const QByteArray &rawText, const QByteArray &key, const QByteArray &iv)
{
    if (m_mode >= CBC && (iv.isNull() || iv.size() != m_blocklen))
       return QByteArray();

    QByteArray ret;
    QByteArray expandedKey = expandKey(key);
    QByteArray ivTemp(iv);

    //Preparation for CFB
    if (m_mode == CFB)
        ret.append(byteXor(rawText.mid(0, m_blocklen), cipher(expandedKey, iv)));

    for(int i=0; i < rawText.size(); i+= m_blocklen){
        switch(m_mode)
        {
        case ECB:
            ret.append(invCipher(expandedKey, rawText.mid(i, m_blocklen)));
            break;
        case CBC:
            ret.append(invCipher(expandedKey, rawText.mid(i, m_blocklen)));
            ret.replace(i, m_blocklen, byteXor(ret.mid(i, m_blocklen),ivTemp));
            ivTemp = rawText.mid(i, m_blocklen);
            break;
        case CFB:
            if (i+m_blocklen < rawText.size()){
                ret.append(byteXor(rawText.mid(i+m_blocklen, m_blocklen),
                                   cipher(expandedKey, rawText.mid(i, m_blocklen))));
            }
            break;
        default:
            //do nothing
            break;
        }
    }
    return ret;
}

QByteArray QAESEncryption::removePadding(const QByteArray &rawText)
{
    QByteArray ret(rawText);
    switch (m_padding)
    {
    case PADDING::ZERO:
        //Works only if the last byte of the decoded array is not zero
        while (ret.at(ret.length()-1) == 0x00)
            ret.remove(ret.length()-1, 1);
        break;
    case PADDING::PKCS7:
        ret.remove(ret.length() - ret.at(ret.length()-1), ret.at(ret.length()-1));
        break;
    case PADDING::ISO:
        ret.truncate(ret.lastIndexOf(0x80));
        break;
    default:
        //do nothing
        break;
    }
    return ret;
}
