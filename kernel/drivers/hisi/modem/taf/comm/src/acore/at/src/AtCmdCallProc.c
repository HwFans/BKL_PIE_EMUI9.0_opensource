/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "AtCmdCallProc.h"

#include "AtSndMsg.h"
#include "ATCmdProc.h"


/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_CALL_PROC_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


VOS_UINT32 At_RcvVcMsgDtmfDecoderIndProc(
    MN_AT_IND_EVT_STRU                 *pstData
)
{
    APP_VC_DTMF_DECODER_IND_STRU       *pstDtmfInd;
    VOS_UINT8                           ucIndex;
    VOS_CHAR                            aucOutput[2];

    /* ͨ��clientid��ȡindex */
    if (AT_FAILURE == At_ClientIdToUserId(pstData->clientId, &ucIndex))
    {
        AT_WARN_LOG("At_RcvVcMsgDtmfDecoderIndProc:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstDtmfInd = (APP_VC_DTMF_DECODER_IND_STRU *)pstData->aucContent;
    aucOutput[0] = pstDtmfInd->ucDtmfCode;
    aucOutput[1] = '\0';

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^DDTMF: %s%s",
                                                    gaucAtCrLf,
                                                    aucOutput,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

VOS_UINT32 AT_CheckCfshNumber(
    VOS_UINT8                          *pucAtPara,
    VOS_UINT16                          usLen
)
{
    VOS_UINT16      ucLoop;

    /* ���볤����Ч���ж�:+�ſ�ͷ�Ĺ��ʺ��룬��󳤶Ȳ��ܴ���33�������ܴ���32 */
    if ('+' == pucAtPara[0])
    {
        if (usLen > (TAF_CALL_MAX_FLASH_DIGIT_LEN + 1))
        {
            return VOS_ERR;
        }

        pucAtPara++;
        usLen--;
    }
    else
    {
        if (usLen > TAF_CALL_MAX_FLASH_DIGIT_LEN)
        {
            return VOS_ERR;
        }
    }

    /* �����ַ���Ч���ж�(���������ʺ�������ַ�'+') */
    for (ucLoop = 0; ucLoop < usLen; ucLoop++)
    {
        if (  ((pucAtPara[ucLoop] >= '0') && (pucAtPara[ucLoop] <= '9'))
            || (pucAtPara[ucLoop] == '*')
            || (pucAtPara[ucLoop] == '#'))
        {
            continue;
        }
        else
        {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 AT_SetCfshPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    TAF_CALL_FLASH_PARA_STRU            stFlashPara;

    /* �������� */
    if(gucAtParaIndex > 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ��Я���˲���<number>���������Ч�� */
    if (1 == gucAtParaIndex)
    {
        if (VOS_OK != AT_CheckCfshNumber(gastAtParaList[0].aucPara,
                                         gastAtParaList[0].usParaLen))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        /* ����AT����AT^CFSH= ���ز������� */
        if(AT_CMD_OPT_SET_PARA_CMD == g_stATParseCmd.ucCmdOptType)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    TAF_MEM_SET_S(&stFlashPara, sizeof(stFlashPara), 0x00, sizeof(TAF_CALL_FLASH_PARA_STRU));

    stFlashPara.ucDigitNum = (VOS_UINT8)gastAtParaList[0].usParaLen;
    TAF_MEM_CPY_S(stFlashPara.aucDigit, sizeof(stFlashPara.aucDigit), gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);

    /* ����TAF_CALL_APP_SEND_FLASH_REQ��Ϣ */
    ulRst = TAF_XCALL_SendFlashReq(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   &stFlashPara);
    if (VOS_OK == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CFSH_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_VOID AT_RcvTafCallSndFlashRslt(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    VOS_UINT8                           ucIndex;
    TAF_CALL_EVT_SEND_FLASH_RSLT_STRU  *pstSndFlashRslt;

    /* ����ClientID��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallSndFlashRslt: Get Index Fail!");
        return;
    }

    /* ATģ���ڵȴ�^CFSH����Ĳ�������¼��ϱ� */
    if (AT_CMD_CFSH_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvTafCallSndFlashRslt: Error Option!");
        return;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ʱ��Ӧ�Ĵ������ӡ����Ľ�� */
    pstSndFlashRslt = (TAF_CALL_EVT_SEND_FLASH_RSLT_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));
    if (VOS_OK == pstSndFlashRslt->ucResult)
    {
        At_FormatResultData(ucIndex, AT_OK);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }

    return;
}


VOS_UINT32 At_TestCBurstDTMFPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16      usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^CBURSTDTMF: (1,2),(0-9,*,#),(95,150,200,250,300,350),(60,100,150,200)");
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetCBurstDTMFPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    TAF_CALL_BURST_DTMF_PARA_STRU       stBurstDtmfPara;
    VOS_UINT16                          ucLoop;

    /*������Ч�Լ��*/
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((gucAtParaIndex != 4)
        ||(0 == gastAtParaList[0].usParaLen)
        ||(0 == gastAtParaList[1].usParaLen)
        ||(0 == gastAtParaList[2].usParaLen)
        ||(0 == gastAtParaList[3].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key������Ч���ж� */
    if (gastAtParaList[1].usParaLen > TAF_CALL_MAX_BURST_DTMF_NUM)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key��Ч���ж� */
    for (ucLoop = 0; ucLoop < gastAtParaList[1].usParaLen; ucLoop++)
    {
        if (  ((gastAtParaList[1].aucPara[ucLoop] >= '0') && (gastAtParaList[1].aucPara[ucLoop] <= '9'))
            || (gastAtParaList[1].aucPara[ucLoop] == '*')
            || (gastAtParaList[1].aucPara[ucLoop] == '#'))
        {
            continue;
        }
        else
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    TAF_MEM_SET_S(&stBurstDtmfPara, sizeof(stBurstDtmfPara), 0x00, sizeof(stBurstDtmfPara));

    stBurstDtmfPara.ucCallId     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stBurstDtmfPara.ucDigitNum   = (VOS_UINT8)gastAtParaList[1].usParaLen;
    TAF_MEM_CPY_S(stBurstDtmfPara.aucDigit, sizeof(stBurstDtmfPara.aucDigit), gastAtParaList[1].aucPara, stBurstDtmfPara.ucDigitNum);
    stBurstDtmfPara.ulOnLength   = gastAtParaList[2].ulParaValue;
    stBurstDtmfPara.ulOffLength  = gastAtParaList[3].ulParaValue;

    /* ����TAF_CALL_APP_SEND_BURST_DTMF_REQ��Ϣ */
    ulRst = TAF_XCALL_SendBurstDtmf(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                   &stBurstDtmfPara);
    if (VOS_OK == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CBURSTDTMF_SET;

        /* �������������״̬ */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_RcvTafCallSndBurstDTMFCnf(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    VOS_UINT8                                               ucIndex;
    TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU                  *pstBurstDtmfCnf;

    /* ����ClientID��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallSndBurstDTMFCnf: Get Index Fail!");
        return VOS_ERR;
    }

    /* ATģ���ڵȴ�^CBURSTDTMF��������Ĳ�������¼��ϱ� */
    if ( AT_CMD_CBURSTDTMF_SET != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvTafCallSndBurstDTMFCnf: Error Option!");
        return VOS_ERR;
    }

    /* ʹ��AT_STOP_TIMER_CMD_READY�ָ�AT����ʵ��״̬ΪREADY״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* ������ʱ��Ӧ�Ĵ������ӡ����Ľ�� */
    pstBurstDtmfCnf = (TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));
    if (TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_SUCCESS != pstBurstDtmfCnf->enResult)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallSndBurstDTMFRslt(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallCalledNumInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_CALLED_NUM_INFO_IND_STRU                  *pstCalledNum;
    VOS_UINT8                                               ucIndex;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM + 1];

    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCalledNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }


    /* ��ʼ�� */
    pstCalledNum = (TAF_CALL_EVT_CALLED_NUM_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstCalledNum->aucDigit, pstCalledNum->ucDigitNum);

    /* ��pstCalledNum->aucDigit�����һλ��'\0',��ֹ��pstCalledNum->aucDigit�޽�����������AT���ϱ� */
    aucDigit[pstCalledNum->ucDigitNum] = '\0';

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCALLEDNUM].pucText,
                                                    pstCalledNum->enNumType,
                                                    pstCalledNum->enNumPlan,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallCallingNumInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_CALLING_NUM_INFO_IND_STRU                 *pstCallingNum;
    VOS_UINT8                                               ucIndex;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];

    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCallingNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstCallingNum = (TAF_CALL_EVT_CALLING_NUM_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));

    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstCallingNum->aucDigit, pstCallingNum->ucDigitNum);

    /* ��pstCallingNum->aucDigit�����һλ��'\0',��ֹ��pstCallingNum->aucDigit�޽�����������AT���ϱ� */
    aucDigit[pstCallingNum->ucDigitNum] = '\0';

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCALLINGNUM].pucText,
                                                    pstCallingNum->enNumType,
                                                    pstCallingNum->enNumPlan,
                                                    pstCallingNum->ucPi,
                                                    pstCallingNum->ucSi,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallDispInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_DISPLAY_INFO_IND_STRU  *pstDisplayInfo;
    VOS_UINT8                            ucIndex;
    VOS_UINT8                            aucDigit[TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM + 1];

    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallDispInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstDisplayInfo = (TAF_CALL_EVT_DISPLAY_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstDisplayInfo->aucDigit, pstDisplayInfo->ucDigitNum);

    /* ��pstDisplayInfo->aucDigit�����һλ��'\0',��ֹ��pstDisplayInfo->aucDigit�޽�����������AT���ϱ� */
    aucDigit[pstDisplayInfo->ucDigitNum] = '\0';

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%s,,,%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CDISP].pucText,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallExtDispInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_EXT_DISPLAY_INFO_IND_STRU                 *pstExtDispInfo;
    VOS_UINT8                                               ucIndex;
    VOS_UINT32                                              ulLoop;
    VOS_UINT32                                              ulDigitNum;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM + 1];

    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallExtDispInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstExtDispInfo = (TAF_CALL_EVT_EXT_DISPLAY_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    for (ulLoop = 0; ulLoop < pstExtDispInfo->ucInfoRecsDataNum; ulLoop++)
    {
        /* ��pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit�����һλ��'\0',
            ��ֹ��pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit�޽�����������AT���ϱ� */
        ulDigitNum = pstExtDispInfo->aucInfoRecsData[ulLoop].ucDigitNum;
        TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
        TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit, ulDigitNum);
        aucDigit[ulDigitNum] = '\0';

        /* �����ѯ��� */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s%s%s,%d,%d,%d%s",
                                                        gaucAtCrLf,
                                                        gastAtStringTab[AT_STRING_CDISP].pucText,
                                                        aucDigit,
                                                        pstExtDispInfo->ucExtDispInd,
                                                        pstExtDispInfo->ucDisplayType,
                                                        pstExtDispInfo->aucInfoRecsData[ulLoop].ucDispalyTag,
                                                        gaucAtCrLf);

        At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);
    }



    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallConnNumInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_CONN_NUM_INFO_IND_STRU *pstConnNumInfo;
    VOS_UINT8                            ucIndex;
    VOS_UINT8                            aucDigit[TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM + 1];


    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallConnNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstConnNumInfo = (TAF_CALL_EVT_CONN_NUM_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstConnNumInfo->aucDigit, pstConnNumInfo->ucDigitNum);

    /* ��pstConnNumInfo->aucDigit�����һλ��'\0',��ֹ��pstConnNumInfo->aucDigit�޽�����������AT���ϱ� */
    aucDigit[pstConnNumInfo->ucDigitNum] = '\0';

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d,%d,%s%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CCONNNUM].pucText,
                                                    pstConnNumInfo->enNumType,
                                                    pstConnNumInfo->enNumPlan,
                                                    pstConnNumInfo->ucPi,
                                                    pstConnNumInfo->ucSi,
                                                    aucDigit,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallRedirNumInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_REDIR_NUM_INFO_IND_STRU                   *pstRedirNumInfo;
    VOS_UINT8                                               ucIndex;
    VOS_UINT16                                              usLength;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM + 1];

    usLength        = 0;


    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallRedirNumInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstRedirNumInfo = (TAF_CALL_EVT_REDIR_NUM_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstRedirNumInfo->aucDigitNum, pstRedirNumInfo->ucDigitNum);

    /* ��pstRedirNumInfo->aucDigitNum�����һλ��'\0',��ֹ��pstRedirNumInfo->aucDigitNum�޽�����������AT���ϱ� */
    aucDigit[pstRedirNumInfo->ucDigitNum] = '\0';

    /* �����ѯ���������EXTENSIONBIT1��EXTENSIONBIT2�����ѡ�� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s%d,%d,%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_CREDIRNUM].pucText,
                                       pstRedirNumInfo->enNumType,
                                       pstRedirNumInfo->enNumPlan,
                                       aucDigit);

    if (VOS_TRUE == pstRedirNumInfo->bitOpPi)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->ucPi);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (VOS_TRUE == pstRedirNumInfo->bitOpSi)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->ucSi);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (VOS_TRUE == pstRedirNumInfo->bitOpRedirReason)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstRedirNumInfo->ucRedirReason);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s",
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallSignalInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_SIGNAL_INFO_IND_STRU   *pstsignalInfo;
    VOS_UINT8                            ucIndex;

    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallSignalInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstsignalInfo = (TAF_CALL_EVT_SIGNAL_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s%s%d,%d,%d%s",
                                                    gaucAtCrLf,
                                                    gastAtStringTab[AT_STRING_CSIGTONE].pucText,
                                                    pstsignalInfo->ucSignalType,
                                                    pstsignalInfo->ucAlertPitch,
                                                    pstsignalInfo->ucSignal,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallLineCtrlInfoInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_LINE_CTRL_INFO_IND_STRU                   *pstLineCtrlInfo;
    VOS_UINT16                                              usLength;
    VOS_UINT8                                               ucIndex;

    usLength = 0;

    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallLineCtrlInfoInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstLineCtrlInfo = (TAF_CALL_EVT_LINE_CTRL_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    /* �����ѯ��� */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                    (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                    "%s%s%d",
                                    gaucAtCrLf,
                                    gastAtStringTab[AT_STRING_CLCTR].pucText,
                                    pstLineCtrlInfo->ucPolarityIncluded);

    if (VOS_TRUE == pstLineCtrlInfo->ucToggleModePresent)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstLineCtrlInfo->ucToggleMode);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    if (VOS_TRUE == pstLineCtrlInfo->ucReversePolarityPresent)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstLineCtrlInfo->ucReversePolarity);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",");
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       ",%d%s",
                                       pstLineCtrlInfo->ucPowerDenialTime,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallCCWACInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_CCWAC_INFO_IND_STRU   *pstCCWAC;
    VOS_UINT8                           ucIndex;
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];

    usLength = 0;

    /* ����clientId��ȡͨ������ */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCCWACInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* ��ʼ�� */
    pstCCWAC = (TAF_CALL_EVT_CCWAC_INFO_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstCCWAC->aucDigit, pstCCWAC->ucDigitNum);

    /* ��pstCCWAC->aucDigit�����һλ��'\0',��ֹ��pstCCWAC->aucDigit�޽�����������AT���ϱ� */
    aucDigit[pstCCWAC->ucDigitNum] = '\0';


    /* �����ѯ��� */
    if (VOS_TRUE == pstCCWAC->ucSignalIsPresent)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%s%s,%d,%d,%d,%d,%d,%d,%d,%d%s",
                                            gaucAtCrLf,
                                            gastAtStringTab[AT_STRING_CCWAC].pucText,
                                            aucDigit,
                                            pstCCWAC->ucPi,
                                            pstCCWAC->ucSi,
                                            pstCCWAC->enNumType,
                                            pstCCWAC->enNumPlan,
                                            pstCCWAC->ucSignalIsPresent,
                                            pstCCWAC->ucSignalType,
                                            pstCCWAC->ucAlertPitch,
                                            pstCCWAC->ucSignal,
                                            gaucAtCrLf);

    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%s%s,%d,%d,%d,%d,%d,,,%s",
                                            gaucAtCrLf,
                                            gastAtStringTab[AT_STRING_CCWAC].pucText,
                                            aucDigit,
                                            pstCCWAC->ucPi,
                                            pstCCWAC->ucSi,
                                            pstCCWAC->enNumType,
                                            pstCCWAC->enNumPlan,
                                            pstCCWAC->ucSignalIsPresent,
                                            gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}


VOS_UINT32 At_TestCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT16                          usLength;

    usLength = 0;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "^CCONTDTMF: (1,2),(0,1),(0-9,*,#)");
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_SetCContinuousDTMFPara(
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          ulRst;
    TAF_CALL_CONT_DTMF_PARA_STRU        stContDtmfPara;

    /* Check the validity of parameter */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Non set command!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  Check the validity of <Call_ID> and <Switch> */
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen))
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Call_ID> or <Switch>!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* If the <Switch> is Start and the number of parameter isn't equal to 3.
       Or if the <Switch> is Stop and the number of parameter isn't equal to 2��both invalid */
    if (((TAF_CALL_CONT_DTMF_STOP == gastAtParaList[1].ulParaValue)
      && (AT_CCONTDTMF_PARA_NUM_MIN != gucAtParaIndex))
     || ((TAF_CALL_CONT_DTMF_START == gastAtParaList[1].ulParaValue)
      && (AT_CCONTDTMF_PARA_NUM_MAX != gucAtParaIndex)))
    {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: The number of parameters mismatch!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* If the <Switch> is Start,the <Dtmf_Key> should be setted and check its validity */
    if (TAF_CALL_CONT_DTMF_START == gastAtParaList[1].ulParaValue)
    {
        if (VOS_ERR == AT_CheckCContDtmfKeyPara())
        {
            AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Dtmf_Key>!");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    TAF_MEM_SET_S(&stContDtmfPara, sizeof(stContDtmfPara), 0x00, sizeof(stContDtmfPara));
    stContDtmfPara.ucCallId     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stContDtmfPara.enSwitch     = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    stContDtmfPara.ucDigit      = (VOS_UINT8)gastAtParaList[2].aucPara[0];

    /* Send TAF_CALL_APP_SEND_CONT_DTMF_REQ Message */
    ulRst = TAF_XCALL_SendContinuousDtmf(gastAtClientTab[ucIndex].usClientId,
                                         gastAtClientTab[ucIndex].opId,
                                        &stContDtmfPara);
    if (VOS_OK == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CCONTDTMF_SET;

        /* Return hang-up state */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}


VOS_UINT32 AT_CheckCContDtmfKeyPara(VOS_VOID)
{
    if (1 != gastAtParaList[2].usParaLen)
    {
        return VOS_ERR;
    }

    if ((('0' <= gastAtParaList[2].aucPara[0]) && ('9' >= gastAtParaList[2].aucPara[0]))
      || ('*' == gastAtParaList[2].aucPara[0])
      || ('#' == gastAtParaList[2].aucPara[0]))
    {
        return VOS_OK;
    }
    else
    {
        return VOS_ERR;
    }
}


VOS_UINT32 AT_RcvTafCallSndContinuousDTMFCnf(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    VOS_UINT8                                               ucIndex;
    TAF_CALL_EVT_SEND_CONT_DTMF_CNF_STRU                   *pstContDtmfCnf = VOS_NULL_PTR;

    /* According to ClientID to get the index */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallSndContinuousDTMFCnf: Get Index Fail!");
        return VOS_ERR;
    }

    /* AT module is waiting for report the result of ^CCONTDTMF command */
    if (AT_CMD_CCONTDTMF_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvTafCallSndContinuousDTMFCnf: Error Option!");
        return VOS_ERR;
    }

    /* Use AT_STOP_TIMER_CMD_READY to recover the AT command state to READY state */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* According to the error code of temporary respond, printf the result of command */
    pstContDtmfCnf = (TAF_CALL_EVT_SEND_CONT_DTMF_CNF_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));
    if (TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_SUCCESS != pstContDtmfCnf->enResult)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallSndContinuousDTMFRslt(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallRcvContinuousDtmfInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_RCV_CONT_DTMF_IND_STRU                    *pstRcvContDtmf = VOS_NULL_PTR;
    VOS_UINT8                                               aucDigit[2];
    VOS_UINT8                                               ucIndex;

    /* According to ClientID to get the index */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallRcvContinuousDtmfInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* initialization */
    pstRcvContDtmf = (TAF_CALL_EVT_RCV_CONT_DTMF_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    /*  Initialize aucDigit[0] with pstRcvContDtmf->ucDigit and  aucDigit[1] = '\0'
        Because At_sprintf does not allow to print pstRcvContDtmf->ucDigit with %c
        Hence, need to convert digit into string and print as string */
    aucDigit[0] = pstRcvContDtmf->ucDigit;
    aucDigit[1] = '\0';

    /* Output the inquire result */
    if (TAF_CALL_CONT_DTMF_START == pstRcvContDtmf->enSwitch)
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s^CCONTDTMF: %d,%d,\"%s\"%s",
                                                        gaucAtCrLf,
                                                        pstRcvContDtmf->ucCallId,
                                                        pstRcvContDtmf->enSwitch,
                                                        aucDigit,
                                                        gaucAtCrLf);
    }
    else
    {
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        "%s^CCONTDTMF: %d,%d%s",
                                                        gaucAtCrLf,
                                                        pstRcvContDtmf->ucCallId,
                                                        pstRcvContDtmf->enSwitch,
                                                        gaucAtCrLf);
    }

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_UINT32 AT_RcvTafCallRcvBurstDtmfInd(
    MN_AT_IND_EVT_STRU                 *pEvtInfo
)
{
    TAF_CALL_EVT_RCV_BURST_DTMF_IND_STRU                   *pstRcvBurstDtmf = VOS_NULL_PTR;
    VOS_UINT8                                               ucIndex;
    VOS_UINT8                                               aucDigit[TAF_CALL_MAX_BURST_DTMF_NUM + 1];

    /* According to ClientID to get the index */
    if(AT_FAILURE == At_ClientIdToUserId(pEvtInfo->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallRcvBurstDtmfInd: Get Index Fail!");
        return VOS_ERR;
    }

    /* initialization */
    pstRcvBurstDtmf = (TAF_CALL_EVT_RCV_BURST_DTMF_IND_STRU *)(pEvtInfo->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));

    TAF_MEM_SET_S(aucDigit, sizeof(aucDigit), 0x00, sizeof(aucDigit));
    TAF_MEM_CPY_S(aucDigit, sizeof(aucDigit), pstRcvBurstDtmf->aucDigit, pstRcvBurstDtmf->ucDigitNum);

    /* Add the '\0' to the last byte of pstRcvBurstDtmf->aucDigit */
    aucDigit[pstRcvBurstDtmf->ucDigitNum] = '\0';

    /* Output the inquire result */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^CBURSTDTMF: %d,\"%s\",%d,%d%s",
                                                    gaucAtCrLf,
                                                    pstRcvBurstDtmf->ucCallId,
                                                    aucDigit,
                                                    pstRcvBurstDtmf->ulOnLength,
                                                    pstRcvBurstDtmf->ulOffLength,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}


VOS_VOID AT_RcvTafCallCclprCnf(MN_AT_IND_EVT_STRU *pstData)
{
    TAF_CALL_EVT_CCLPR_GET_CNF_STRU    *pstCClprGetCnf;
    VOS_UINT16                          usLength;
    VOS_UINT8                           ucIndex;

    /* ��ʼ�� */
    ucIndex = 0;

    /* ͨ��ClientId��ȡucIndex */
    if(AT_FAILURE == At_ClientIdToUserId(pstData->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCclprCnf: WARNING:AT INDEX NOT FOUND!");
        return;
    }

    /* ���Ϊ�㲥���ͣ��򷵻�AT_ERROR */
    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafCallCclprCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* �жϵ�ǰ���������Ƿ�ΪAT_CMD_CCLPR_GET */
    if (AT_CMD_CCLPR_SET != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvTafCallCclprCnf: WARNING:Not AT_CMD_CCLPR_GET!");
        return;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstCClprGetCnf = (TAF_CALL_EVT_CCLPR_GET_CNF_STRU *)(pstData->aucContent
                                              + sizeof(MN_CALL_EVENT_ENUM_U32));
    /* �жϲ�ѯ�����Ƿ�ɹ� */
    if (TAF_ERR_NO_ERROR != pstCClprGetCnf->ulRet)
    {
        At_FormatResultData(ucIndex, AT_ERROR);
        return;
    }

    usLength = 0;

    if (TAF_CALL_PRESENTATION_BUTT != pstCClprGetCnf->enPI)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: %d",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstCClprGetCnf->enPI);

    }
    /* ��ӡ��� */
    gstAtSendData.usBufLen  = usLength;
    At_FormatResultData(ucIndex, AT_OK);

    return;

}


VOS_UINT32 AT_TestCclprPara( VOS_UINT8 ucIndex )
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-2)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetCclprPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    /* ������� */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����������� */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ���Ϳ����ϢTAF_CALL_APP_SND_CCLPR_REQ��C��,  */
    ulResult = TAF_XCALL_SendCclpr(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                  (MN_CALL_ID_T)gastAtParaList[0].ulParaValue);
    if (VOS_OK != ulResult)
    {
        AT_WARN_LOG("AT_SetCclprPara: TAF_XCALL_SendCclpr fail.");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CCLPR_SET;

    return AT_WAIT_ASYNC_RETURN;

}

VOS_UINT32 AT_SetRejCallPara(VOS_UINT8 ucIndex)
{
    MN_CALL_SUPS_PARAM_STRU             stCallRejParam;

     /* ָ�����ͼ�� */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetRejCallPara : Current Option is not AT_CMD_REJCALL!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ������� */
    if (2 != gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SetRejCallPara : The number of input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ����Ϊ�� */
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen))
    {
        AT_WARN_LOG("AT_SetRejCallPara : Input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    TAF_MEM_SET_S(&stCallRejParam, sizeof(stCallRejParam), 0x00, sizeof(stCallRejParam));

    stCallRejParam.enCallSupsCmd  = MN_CALL_SUPS_CMD_REL_INCOMING_OR_WAITING;
    stCallRejParam.callId         = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stCallRejParam.enCallRejCause = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    if (VOS_OK != MN_CALL_Sups(gastAtClientTab[ucIndex].usClientId, 0, &stCallRejParam))
    {
        AT_WARN_LOG("AT_SetRejCallPara : Send Msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_REJCALL_SET;

    return AT_WAIT_ASYNC_RETURN;    /* �������������״̬ */
}


VOS_UINT32 AT_TestRejCallPara( VOS_UINT8 ucIndex )
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (0,1)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_QryCimsErrPara(VOS_UINT8 ucIndex)
{
    TAF_CALL_ERROR_INFO_TEXT_STRU      *pstCallErrInfo = VOS_NULL_PTR;

    pstCallErrInfo = AT_GetCallErrInfoText(ucIndex);

    /* �����ѯ��� */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %d,\"%s\"",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          AT_GetCsCallErrCause(ucIndex),
                                          pstCallErrInfo->acErrInfoText);

    return AT_OK;
}


VOS_UINT32 AT_QryCsChannelInfoPara( VOS_UINT8 ucIndex )
{
    VOS_UINT32                          ulRst;

    /* �����첽Ӧ������ */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_TAF_APP_CHANNEL_INFO_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_WUEPS_PID_TAF);

    if (VOS_OK != ulRst)
    {
        AT_WARN_LOG("AT_QryCsChannelInfoPara: Send Msg fail!");
        return AT_ERROR;
    }

    /* ����ATģ��ʵ���״̬Ϊ�ȴ��첽���� */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CSCHANNELINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_RcvTafSpmQryCSChannelInfoCnf(
    MN_AT_IND_EVT_STRU                 *pstData
)
{
    TAF_APP_CHANNEL_INFO_QRY_CNF_STRU  *pstChannelInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    ucIndex        = 0;

    /* ����clientId��ȡͨ������ */
    if (AT_FAILURE == At_ClientIdToUserId(pstData->clientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafQryChannelCnf: Get Index Fail!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvTafSpmQryChannelInfoCnf: WARNING:AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

     /* ��ǰAT�Ƿ��ڵȴ�������� */
    if (AT_CMD_CSCHANNELINFO_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        return VOS_ERR;
    }

    /* ��λAT״̬ */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    pstChannelInfo = (TAF_APP_CHANNEL_INFO_QRY_CNF_STRU *)(pstData->aucContent + sizeof(MN_CALL_EVENT_ENUM_U32));

    if (VOS_OK != pstChannelInfo->ulResult)
    {
        ulResult = AT_ERROR;
        gstAtSendData.usBufLen = 0;
    }
    else
    {
        ulResult = AT_OK;

        /* �����ѯ��� */
        gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s: %d,%d",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstChannelInfo->enChannelType,
                                          pstChannelInfo->enVoiceDomain);
    }

    /* ����At_FormatResultData���������� */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

