<?xml version="1.0"?>
<!--
  Мостик для консольного интерпретатора
  Параметры:
    * ParamsFile - имя файла с параметрами
    * SessionFile - имя файла с сеансом
    * HelperFunction - функция
  Функции:
    * Load - чтение файлов, запись параметров и сеанса
    * Save - чтение сеанса и запись в файл  (параметры не записываются никогда; файл сеанса должен существовать)
  ss, 11.2013
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:define name="error">
    <xpl:fatal/>
  </xpl:define>

  <xpl:define name="CheckAbsPath">
    <xpl:choose>
      <xpl:when>
        <xpl:test>
          <xpl:file-exists>
            <xpl:attribute name="file">
              <xpl:content/>
            </xpl:attribute>
          </xpl:file-exists>()
        </xpl:test>
        <xpl:text>false</xpl:text>
      </xpl:when>
      <xpl:when>
        <xpl:test>
         '<xpl:file-exists abspath="true">
            <xpl:attribute name="file">
              <xpl:content/>
            </xpl:attribute>
          </xpl:file-exists>' = 'true'
        </xpl:test>
        <xpl:text>true</xpl:text>
      </xpl:when>
      <xpl:otherwise>
        <xpl:debug-print severity="error">Cannot locate file <xpl:content/></xpl:debug-print>
        <xpl:fatal/>
      </xpl:otherwise>
    </xpl:choose>
  </xpl:define>

  <xpl:define name="Load">
    <xpl:define name="IncludeAuxFile">
      <xpl:include select="/*/node()">
        <xpl:attribute name="abspath">
          <CheckAbsPath>
            <xpl:content/>
          </CheckAbsPath>
        </xpl:attribute>
        <xpl:attribute name="file">
          <xpl:content/>
        </xpl:attribute>
      </xpl:include> 
    </xpl:define>

    <xpl:if>
      <xpl:test>'<xpl:get-param name="ParamsFile" expect="any"/>' != ''</xpl:test>
      <xpl:define name="param" id="param">
        <xpl:define name="Value">
          <xpl:set-param mode="add">
            <xpl:attribute name="name">
              <xpl:content select="@name" id="param"/>
            </xpl:attribute>
            <xpl:content/>
          </xpl:set-param>
        </xpl:define>
        <xpl:content/>
      </xpl:define>
   
      <IncludeAuxFile>
        <xpl:get-param name="ParamsFile" expect="any"/>
      </IncludeAuxFile>  
    </xpl:if>

    <xpl:if>
      <xpl:test>'<xpl:get-param name="SessionFile" expect="any"/>' != ''</xpl:test>
      <xpl:container returncontent="false">
        <IncludeAuxFile>
          <xpl:get-param name="SessionFile" expect="any"/>
        </IncludeAuxFile>

        <xpl:with select="preceding-sibling::*">
          <xpl:session-set-object>
            <xpl:attribute name="name">
              <xpl:content select="local-name()"/>
            </xpl:attribute>
            <xpl:content/>
          </xpl:session-set-object>
        </xpl:with>
      </xpl:container>
    </xpl:if>
  </xpl:define>
 
  <xpl:define name="Save">
    <xpl:define name="SessionFile" expand="true">
      <xpl:get-param name="SessionFile" expect="any"/>
    </xpl:define>

    <xpl:save encoding="utf-8"> <!-- preserve any symbols beyond 8-bit encodings -->
      <xpl:attribute name="file">
        <SessionFile/>
      </xpl:attribute>
      <xpl:attribute name="abspath"> <!-- we suppose the file exists (touch xxx.xml), otherwise xpl:fatal will be called (todo?) -->
        <CheckAbsPath>
          <SessionFile/>
        </CheckAbsPath>
      </xpl:attribute>
      <xpl:session-get-object/>
    </xpl:save>
  </xpl:define>

  <xpl:define name="HelperFunction" expand="true">
    <xpl:get-param name="HelperFunction" expect="any"/>
  </xpl:define>

  <xpl:if>
    <xpl:test>
      '<xpl:is-defined>
        <xpl:attribute name="name">
          <HelperFunction/>
        </xpl:attribute>
      </xpl:is-defined>' = 'true'
    </xpl:test>
    <xpl:element>
      <xpl:attribute name="name">
        <HelperFunction/>
      </xpl:attribute>
    </xpl:element>
  </xpl:if>
</Root>
