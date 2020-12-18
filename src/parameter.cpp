/*
*  Copyright 2020 José María Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/MorphogenGL>
*
*  This file is part of MorphogenGL.
*
*  MorphogenGL is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  MorphogenGL is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "parameter.h"

void FloatParameter::setValue(float theValue)
{
    value = theValue;
    operation->setFloatParameter(index, value);
}

void MatrixParameter::setValues()
{
    GLfloat* elements = &values[0];
    operation->setMatrixParameter(elements);
}

void KernelParameter::setValues()
{
    GLfloat* elements = &values[0];
    operation->setKernelParameter(elements);
}