/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AVRUSI_H
#define AVRUSI_H

#include "mcumodule.h"

class McuPin;

class AvrUsi : public McuModule
{
    public:
        AvrUsi( eMcu* mcu, QString name );
        ~AvrUsi();

        virtual void reset() override;
        virtual void configureA( uint8_t ) override;
        virtual void configureB( uint8_t ) override;

    private:
        inline void stepCounter();
        inline void softCounter( bool clk, bool toggle );
        inline void shiftData();
        inline void toggleClock();

        bool m_softClk;
        bool m_clkEdge;

        uint8_t* m_dataReg;
        uint8_t* m_bufferReg;
        uint8_t* m_statusReg;

        uint8_t m_mode;
        uint8_t m_clockMode;
        uint8_t m_counter;

        // USICR
        regBits_t m_USITC;
        regBits_t m_USICLK;
        regBits_t m_USICS;
        regBits_t m_USIWM;

        // USISR
        regBits_t m_USICNT;

        McuPin* m_DOpin;
        McuPin* m_DIpin;
        McuPin* m_CKpin;
};

#endif
