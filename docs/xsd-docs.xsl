<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xsd="http://www.w3.org/2001/XMLSchema"
    xmlns:bdem="http://bloomberg.com/schemas/bdem">
  <xsl:output method="html" />

  <!--
      TEMPLATE: match /

      Set up HTML document structure.
    -->
  <xsl:template match="/">
    <html>
      <body>
        <xsl:apply-templates select="/xsd:schema/xsd:simpleType | /xsd:schema/xsd:complexType" />
      </body>
    </html>
  </xsl:template>

  <!--
      TEMPLATE: match xsd:simpleType

      Add a heading describing a simple type, that is, a restriction on a basic
      type like `xsd:string`.  The heading is given an ID built from the prefix
      of the target XML namespace and the name of the type.
    -->
  <xsl:template match="xsd:simpleType">
    <h2>
      <xsl:attribute name="id">
        <xsl:call-template name="prefixedName" />
      </xsl:attribute>
      Simple Type:
      <strong><xsl:call-template name="prefixedName" /></strong>
      (<em><xsl:value-of select="xsd:restriction/@base" /></em>)
    </h2>
    <pre><xsl:apply-templates select="xsd:annotation" /></pre>
    <xsl:apply-templates select="xsd:restriction" />
  </xsl:template>

  <!--
      TEMPLATE: prefixedName

      Concatenate the prefix of the target XML namespace (the `bdem:package`
      attribute on the root element) with the `xsd:name` attribute to form a
      prefixed name.
    -->
  <xsl:template name="prefixedName">
    <xsl:value-of select="/xsd:schema/@bdem:package" />:<xsl:value-of select="@name" />
  </xsl:template>

  <xsl:template match="xsd:restriction">
    <xsl:choose>
      <xsl:when test="@bdem:preserveEnumOrder">
        <ol><xsl:apply-templates select="xsd:enumeration" /></ol>
      </xsl:when>
      <xsl:otherwise>
        <ul><xsl:apply-templates select="xsd:enumeration" /></ul>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="xsd:enumeration">
    <xsl:choose>
      <xsl:when test="@bdem:id">
        <li><code>"<xsl:value-of select="@value" />"</code> = <xsl:value-of select="@value" /></li>
      </xsl:when>
      <xsl:otherwise>
        <xsl:message terminate="no">Missing `bdem:id` on `xsd:enumeration` <xsl:value-of select="@value" />.</xsl:message>
        <li><code>"<xsl:value-of select="@value" />"</code></li>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="xsd:complexType">
    <h2>
      <xsl:attribute name="id"><xsl:value-of select="/xsd:schema/@bdem:package" />:<xsl:value-of select="@name" /></xsl:attribute>
      Complex Type:
      <strong><xsl:value-of select="/xsd:schema/@bdem:package" />:<xsl:value-of select="@name" /></strong>
    </h2>
    <pre><xsl:apply-templates select="xsd:annotation" /></pre>
    { <xsl:apply-templates select="xsd:sequence | xsd:choice" /> }
  </xsl:template>

  <xsl:template match="xsd:sequence">
    <xsd:choose>
      <xsd:when test="count(xsd:element | xsd:choice) > 0">
        <ul>
          <xsl:apply-templates select="xsd:element | xsd:choice">
            <xsl:with-param name="separator">, </xsl:with-param>
          </xsl:apply-templates>
        </ul>
      </xsd:when>
      <xsd:otherwise>
        <p>/* Empty */</p>
      </xsd:otherwise>
    </xsd:choose>
  </xsl:template>

  <xsl:template match="xsd:choice">
    One of:
    <ul>
      <xsl:apply-templates select="xsd:element">
        <xsl:with-param name="separator"> | </xsl:with-param>
      </xsl:apply-templates>
    </ul>
  </xsl:template>

  <xsl:template match="xsd:element">
    <xsl:param name="separator" />

    <li>
      "<xsl:value-of select="@name" />":
      <em>
        <xsl:choose>
          <xsl:when test="starts-with(@type, /xsd:schema/@bdem:package)">
            <a>
              <xsl:attribute name="href">#<xsl:value-of select="@type" /></xsl:attribute>
              <xsl:value-of select="@type" />
            </a>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@type" />
          </xsl:otherwise>
        </xsl:choose>
      </em>
      <xsl:if test="position() != last()">
        <xsl:value-of select="$separator" />
      </xsl:if>
      <xsl:apply-templates select="xsd:annotation" />
    </li>
  </xsl:template>

  <xsl:template match="xsd:annotation">
    <p style="margin-left: 2em;"><xsl:apply-templates /></p>
  </xsl:template>

  <xsl:template match="xsd:documentation">
    /*
    <xsl:apply-templates />
    <xsl:if test="ancestor::xsd:element/@default">
      <br />
      Default: <xsl:value-of select="ancestor::xsd:element/@default" />
    </xsl:if>
    <xsl:if test="ancestor::xsd:element/@minOccurs">
      <br />
      Min Occurrences: <xsl:value-of select="ancestor::xsd:element/@minOccurs" />
    </xsl:if>
    <xsl:if test="ancestor::xsd:element/@maxOccurs">
      <br />
      Max Occurrences: <xsl:value-of select="ancestor::xsd:element/@maxOccurs" />
    </xsl:if>
    */
  </xsl:template>
</xsl:stylesheet>
