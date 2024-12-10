/*

    Code128

    Code 128 is a very high-density barcode symbology. It is used
    for alphanumeric or numeric-only barcodes. This implementation
    can encode all 128 characters of ASCII. The implementation can
    switch between mode B and mode C to compress numbers.

    https://en.wikipedia.org/wiki/Code_128


    The MIT License (MIT)

    Copyright (c) 2013 Promixis, LLC

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", (char*)WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

*/

#include "code128.h"

Code128::Symbol Code128::symbols[108] = {

    (char*)"212222", (char*)"222122", (char*)"222221", (char*)"121223", (char*)"121322", (char*)"131222", (char*)"122213", (char*)"122312", (char*)"132212", (char*)"221213",
    (char*)"221312", (char*)"231212", (char*)"112232", (char*)"122132", (char*)"122231", (char*)"113222", (char*)"123122", (char*)"123221", (char*)"223211", (char*)"221132",
    (char*)"221231", (char*)"213212", (char*)"223112", (char*)"312131", (char*)"311222", (char*)"321122", (char*)"321221", (char*)"312212", (char*)"322112", (char*)"322211",
    (char*)"212123", (char*)"212321", (char*)"232121", (char*)"111323", (char*)"131123", (char*)"131321", (char*)"112313", (char*)"132113", (char*)"132311", (char*)"211313",
    (char*)"231113", (char*)"231311", (char*)"112133", (char*)"112331", (char*)"132131", (char*)"113123", (char*)"113321", (char*)"133121", (char*)"313121", (char*)"211331",
    (char*)"231131", (char*)"213113", (char*)"213311", (char*)"213131", (char*)"311123", (char*)"311321", (char*)"331121", (char*)"312113", (char*)"312311", (char*)"332111",
    (char*)"314111", (char*)"221411", (char*)"431111", (char*)"111224", (char*)"111422", (char*)"121124", (char*)"121421", (char*)"141122", (char*)"141221", (char*)"112214",
    (char*)"112412", (char*)"122114", (char*)"122411", (char*)"142112", (char*)"142211", (char*)"241211", (char*)"221114", (char*)"413111", (char*)"241112", (char*)"134111",
    (char*)"111242", (char*)"121142", (char*)"121241", (char*)"114212", (char*)"124112", (char*)"124211", (char*)"411212", (char*)"421112", (char*)"421211", (char*)"212141",
    (char*)"214121", (char*)"412121", (char*)"111143", (char*)"111341", (char*)"131141", (char*)"114113", (char*)"114311", (char*)"411113", (char*)"411311", (char*)"113141",
    (char*)"114131", (char*)"311141", (char*)"411131", (char*)"211412", (char*)"211214", (char*)"211232", (char*)"2331112", (char*)""
};


Code128::Symbol Code128::symbolCode(quint8 c)
{
    if ( c > SYM_Stop )
    {
        return symbols[107];
    }
    return symbols[c];
}



void Code128::addSymbolCodeChar(char c, QStringList &symbols, int &checksum)
{
    // keep characters in valid range.
    if ( c < 32 )
        c = 32;
    if ( c > 126 )
        c = 32;
    quint8 value = (unsigned int)c - 32;
    Code128::Symbol symbol = symbolCode(value);
    symbols += symbol;
    checksum += value * ( symbols.count() == 1 ? 1 : symbols.count() - 1 );
}

void Code128::addSymbolCodeInt(quint8 value, QStringList &symbols, int &checksum)
{
    Code128::Symbol symbol = symbolCode(value);
    symbols += symbol;
    checksum += value * ( symbols.count() == 1 ? 1 : symbols.count() - 1 );
}

quint8 Code128::digitCount(const QString &data, int startPos)
{
    quint8 cnt = 0;
    for ( ;startPos < data.length();startPos++ )
    {
        if ( !data.at(startPos).isDigit() )
            break;
        cnt++;
    }
    cnt &= 0xfe; // only pairs please.
    return cnt;
}

Code128::BarCode Code128::encode(const QString &data)
{
    QStringList symbols;
    int checkSum = 0;

    if ( data.length() > 100 || data.length() == 0 )
    {
        return BarCode();
    }

    int pos = 0;
    Mode mode = IDLE;
    while ( pos < data.length() )
    {
        quint8 dc = digitCount(data, pos);
        if ( dc >= 4 )
        {
            if ( mode != MODE_C )
            {
                if ( mode == IDLE )
                    addSymbolCodeInt(SYM_StartC, symbols, checkSum);
                else
                    addSymbolCodeInt(SYM_CodeC, symbols, checkSum);
                mode = MODE_C;
            }

            dc = dc>>1;
            for ( int i=0; i < dc; i++ )
            {
                QString v = data.mid(pos,2);
                int value = v.toInt();
                addSymbolCodeInt(value, symbols, checkSum);
                pos+=2;
            }
        }
        else
        {
            if ( mode != MODE_B )
            {
                if ( mode == IDLE )
                    addSymbolCodeInt(SYM_StartB, symbols, checkSum);
                else
                    addSymbolCodeInt(SYM_CodeB, symbols, checkSum);
                mode = MODE_B;
            }
            addSymbolCodeChar( data.at(pos).toLatin1(), symbols, checkSum );
            pos++;
        }
    }

    quint8 remainder = checkSum % 103;
    addSymbolCodeInt(remainder, symbols, checkSum);
    addSymbolCodeInt(SYM_Stop, symbols, checkSum);

    QString code = symbols.join("");
    BarCode b;
    for(int i=0;i<code.length();i++)
    {
        QString v = code.at(i);
        b << v.toInt();
    }
    return b;
}
