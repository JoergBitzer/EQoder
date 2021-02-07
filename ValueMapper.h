/**
 * @file ValueMapper.h
 * @author J. Bitzer @ Jade Hochschule
 * @brief a small header-based class to map values (interpolate) between two points
 * the form is controlled by a val^beta with 0 < beta < oo 
 * @version 0.1
 * @date 2021-02-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once
#include <cmath>
class ValueMapper
{
public:
    ValueMapper()
    {m_x1 = 0.0;m_x2 = 1.0;m_y1 = 0.0;m_y2 = 1.0;m_form = 1.0;computex();computey();};
    ValueMapper(double x1, double y1, double x2, double y2, double form = 0.0)
    {m_x1 = x1;m_x2 = x2;m_y1 = y1;m_y2 = y2;m_form = form;computex();computey();};
    void setx1(double x1) {m_x1 = x1; computex();};
    void setx2(double x2) {m_x2 = x2; computex();};
    void sety1(double y1) {m_y1 = y1; computey();};
    void sety2(double y2) {m_y2 = y2; computey();};
    void setForm(double newForm){m_form = newForm;};

    double getValue (double xin){if (xin > m_x2) xin = m_x2;
        return ( m_diffy*pow(m_inversediffx*(xin-m_x1),m_form) + m_y1);
    };
private:
    void computex(){m_inversediffx = 1.0/(m_x2 -m_x1);};
    void computey(){m_diffy = (m_y2-m_y1); };
    double m_x1;
    double m_y1;
    double m_x2;
    double m_y2;
    double m_form;
    
    double m_inversediffx;
    double m_diffy;

};