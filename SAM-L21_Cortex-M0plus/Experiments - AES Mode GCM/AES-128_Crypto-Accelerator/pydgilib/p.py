# from pydgilib import DGILib
# with DGILib() as dgilib:
#     print(dgilib.device_sn)


from pydgilib_extra import DGILibExtra, LOGGER_PLOT
with DGILibExtra(loggers=[LOGGER_PLOT]) as dgilib:
     dgilib.logger.log(20)    
