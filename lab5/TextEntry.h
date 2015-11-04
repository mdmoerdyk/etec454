extern void TypeText(void);
extern void GetMessage(INT8U *message);
extern void MessageCheckSum(INT8U *message, INT8U sourcea, INT8U sourceb,INT8U sourcec);
extern void TransmitCheck(INT8U *message);
extern void ReceivedCheckSum(INT8U *message, INT8U *rec_mess, INT8U sourcea, INT8U sourceb,INT8U sourcec);
extern void DispTimeStamp(void);
extern void GetReceiveTime(void);