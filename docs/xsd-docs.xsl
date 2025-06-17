<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsd="http://www.w3.org/2001/XMLSchema">
  <xsl:output method="html" />

  <xsl:template match="/">
    <html>
      <body>
        <xsl:apply-templates select="/xsd:schema/xsd:complexType" />
      </body>
    </html>
  </xsl:template>

  <xsl:template match="xsd:complexType">
    Complex Type: <i><xsl:value-of select="@name" /></i>
    <xsl:apply-templates select="xsd:annotation" />
  </xsl:template>

  <xsl:template match="xsd:annotation">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="xsd:documentation">
    <b><xsl:apply-templates /></b>
  </xsl:template>
</xsl:stylesheet>
