<?xml version="1.0" encoding="utf-8"?>
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:define name="ShowFile" expand="true">
    <xpl:debug-print severity="info">== <xpl:get-document-filename/> ==</xpl:debug-print>
  </xpl:define>

  <xpl:define name="MustSucceed" id="MustSucceed">
    <xpl:debug-print severity="info">  Running <xpl:content select="@name"/>...</xpl:debug-print>

    <xpl:define name="error">
      <!-- TODO we need a way to report an error outside (e.g. exit code) -->
      <xpl:debug-print severity="error">   Error: <xpl:content/></xpl:debug-print>
      <xpl:append destination="ancestor::MustSucceed[1]" position="first">
        <Processed>
          <xpl:attribute name="name"><xpl:content select="@name"/></xpl:attribute>
          <Succeeded>false</Succeeded>
          <Error><xpl:content/></Error>
          <xpl:no-expand>
            <xpl:content select="Input" id="MustSucceed"/>
          </xpl:no-expand>
        </Processed>
      </xpl:append>
      <xpl:break point="ancestor::MustSucceed[1]"/>
    </xpl:define>

    <xpl:define name="ProcessedInput" expand="true">
      <xpl:content select="Input/node()" id="MustSucceed"/>
    </xpl:define>

    <xpl:define name="Result" expand="true">
      <xpl:value-of>
        <xpl:attribute name="select">
          '<xpl:serialize><ProcessedInput/></xpl:serialize>' =
          '<xpl:serialize><xpl:content select="Expected/node()" id="MustSucceed"/></xpl:serialize>'
        </xpl:attribute>
      </xpl:value-of>
    </xpl:define>

    <xpl:choose>
      <xpl:when>
        <xpl:test><Result/>()</xpl:test>
        <xpl:debug-print severity="info">    Passed</xpl:debug-print>
      </xpl:when>
      <xpl:otherwise>
        <xpl:debug-print severity="error">   Failed</xpl:debug-print>
      </xpl:otherwise>
    </xpl:choose>

    <Processed>
      <xpl:attribute name="name"><xpl:content select="@name"/></xpl:attribute>
      <Succeeded>
        <Result/>
      </Succeeded>
      <xpl:no-expand>
        <xpl:content select="Input"/>
      </xpl:no-expand>
      <Output>
        <ProcessedInput/>
      </Output>
      <xpl:if>
        <xpl:test>not(<Result/>())</xpl:test>
        <xpl:content select="Expected"/>
      </xpl:if>
    </Processed>
  </xpl:define>

  <xpl:define name="MustFail" id="MustFail">
    <xpl:debug-print severity="info">  Running <xpl:content select="@name"/>...</xpl:debug-print>   

    <Processed>
      <xpl:attribute name="name"><xpl:content select="@name"/></xpl:attribute>
      <Input>
        <xpl:no-expand>
          <xpl:content select="Input"/>
        </xpl:no-expand>
      </Input>
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

  <MustSucceed name="SanityCheck">
    <Input/>
    <Expected/>
  </MustSucceed>
</Root>
