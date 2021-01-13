<?xml version="1.0" encoding="utf-8"?>
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:define name="ShowFile" expand="true">
    <xpl:debug-print severity="info">== <xpl:get-document-filename/> ==</xpl:debug-print>
  </xpl:define>

  <xpl:define name="MustSucceed" id="MustSucceed">
    <xpl:debug-print severity="info">  Running <xpl:content select="@name"/>...</xpl:debug-print>

    <Processed>
      <xpl:attribute name="name"><xpl:content select="@name"/></xpl:attribute>
      <xpl:no-expand>
        <xpl:content select="Input"/>
      </xpl:no-expand>
      <Output>
        <xpl:content select="Input/node()"/>
      </Output>

      <xpl:define name="HasError" expand="true">
        <xpl:value-of select="count(preceding::Output[1]//error)"/>
      </xpl:define>

      <xpl:define name="Match" expand="true">
        <xpl:value-of>
          <xpl:attribute name="select">
            '<xpl:serialize><xpl:include select="preceding::Output[1]/node()"/></xpl:serialize>' =
            '<xpl:serialize><xpl:content select="Expected/node()" id="MustSucceed"/></xpl:serialize>'
          </xpl:attribute>
        </xpl:value-of>
      </xpl:define>

      <xpl:choose>
        <xpl:when>
          <xpl:test><HasError/></xpl:test>
          <Succeeded>false</Succeeded>
          <xpl:debug-print severity="error">   Error: <xpl:include select="preceding::Output[1]//error/text()"/></xpl:debug-print>
        </xpl:when>
        <xpl:when>
          <xpl:test>not(<Match/>())</xpl:test>
          <Succeeded>false</Succeeded>
          <xpl:debug-print severity="error">   Failed</xpl:debug-print>
          <xpl:no-expand>
            <xpl:content select="Expected"/>
          </xpl:no-expand>
        </xpl:when>
        <xpl:otherwise>
          <Succeeded>true</Succeeded>
          <xpl:debug-print severity="info">    Passed</xpl:debug-print>
        </xpl:otherwise>
      </xpl:choose>
    </Processed>
  </xpl:define>

  <xpl:define name="MustFail" id="MustFail">
    <xpl:debug-print severity="info">  Running <xpl:content select="@name"/>...</xpl:debug-print>   

    <Processed>
      <xpl:attribute name="name"><xpl:content select="@name"/></xpl:attribute>
      <xpl:no-expand>
        <xpl:content select="Input"/>
      </xpl:no-expand>
      <Output>
        <xpl:content select="Input/node()"/>
      </Output>
      <xpl:choose>
        <xpl:when>
          <xpl:test>not(ancestor::xpl:choose[1]/preceding-sibling::Output[1]//error)</xpl:test>
          <Succeeded>false</Succeeded>
          <xpl:debug-print severity="error">   Failed: no error thrown</xpl:debug-print>
        </xpl:when>
        <xpl:otherwise>
          <Succeeded>true</Succeeded>
          <xpl:debug-print severity="info">    Passed</xpl:debug-print>
        </xpl:otherwise>
      </xpl:choose>
    </Processed>   
  </xpl:define>

  <xpl:define name="Summary">
    <xpl:define name="ProcessedCount" expand="true">
      <xpl:value-of select="count(ancestor::Summary[1]/preceding-sibling::Processed)"/>
    </xpl:define>
    <xpl:define name="PassedCount" expand="true">
      <xpl:value-of select="count(ancestor::Summary[1]/preceding-sibling::Processed[Succeeded='true'])"/>
    </xpl:define>
    <xpl:define name="FailedCount" expand="true">
      <xpl:value-of select="count(ancestor::Summary[1]/preceding-sibling::Processed[Succeeded='false'])"/>
    </xpl:define>
    <xpl:debug-print severity="info">Total: <ProcessedCount/>, passed: <PassedCount/>, failed: <FailedCount/></xpl:debug-print>
  </xpl:define>

  <MustSucceed name="SanityCheck">
    <Input/>
    <Expected/>
  </MustSucceed>
</Root>
