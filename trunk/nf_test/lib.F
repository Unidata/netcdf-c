    / *Macros for accessing attribute test data */
    /* varid is -1 for NF_GLOBAL so can do global atts in same loop */

      FUNCTION VARID(VID)
      IMPLICIT NONE
      INTEGER VID
#include "tests.inc"
      IF (VID .LT. 0) THEN
          VARID = NF_GLOBAL
      ELSE
          VARID = VID
      ENDIF
      end


      FUNCTION NATTS(VID)
      INTEGER VID
#include "tests.inc"
      IF (VID .LE. 0) THEN
          NATTS = NGATTS
      ELSE
          NATTS = VAR_NATTS(VID)
      ENDIF
      END


      FUNCTION ATT_NAME(J,VID)
      INTEGER J
      INTEGER VID
#include "tests.inc"
      IF (VID .LE. 0) THEN
          ATT_NAME = GATT_NAME(J)
      ELSE
          ATT_NAME = ATTNAME(J,VID)
      ENDIF
      END


      FUNCTION ATT_TYPE(J,VID)
      INTEGER J
      INTEGER VID
#include "tests.inc"
      IF (VID .LE. 0) THEN
          ATT_TYPE = GATT_TYPE(J)
      ELSE
          ATT_TYPE = ATTTYPE(J,VID)
      ENDIF
      END


      FUNCTION ATT_LEN(J,VID)
      INTEGER J
      INTEGER VID
#include "tests.inc"
      IF (VID .LE. 0) THEN
          ATT_LEN = GATT_LEN(J)
      ELSE
          ATT_LEN = ATTLEN(J,VID)
      ENDIF
      END
