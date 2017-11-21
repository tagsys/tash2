from gnuradio import gr
from gnuradio import uhd
from gnuradio import blocks
from gnuradio import filter
from gnuradio import analog
from gnuradio import digital
from gnuradio import qtgui
from PyQt4 import Qt
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import qtgui
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import sip
import sys
import rfid
from gnuradio import qtgui
DEBUG = False
class reader_top_block(gr.top_block, Qt.QWidget):
    # Configure usrp source
    def u_source(self):
        self.source = uhd.usrp_source(
            device_addr=self.usrp_address_source,
            stream_args=uhd.stream_args(
                cpu_format="fc32",
                channels=range(1),
            ),
        )
        self.source.set_samp_rate(self.adc_rate)
        self.source.set_center_freq(self.freq, 0)
        self.source.set_gain(self.rx_gain, 0)
        self.source.set_antenna("RX2", 0)
        self.source.set_auto_dc_offset(False)  # Uncomment this line for SBX daughterboard

    # Configure usrp sink
    def u_sink(self):
        self.sink = uhd.usrp_sink(
            device_addr=self.usrp_address_sink,
            stream_args=uhd.stream_args(
                cpu_format="fc32",
                channels=range(1),
            ),
        )
        self.sink.set_samp_rate(self.dac_rate)
        self.sink.set_center_freq(self.freq, 0)
        self.sink.set_gain(self.tx_gain, 0)
        self.sink.set_antenna("TX/RX", 0)

    def __init__(self):
        gr.top_block.__init__(self)

        self.dac_rate = 1e6  # DAC rate
        self.adc_rate = 100e6 / 50  # ADC rate (2MS/s complex samples)
        self.decim = 5  # Decimation (downsampling factor)
        self.ampl = 0.2  # Output signal amplitude (signal power vary for different RFX900 cards)
        self.freq = 915e6  # Modulation frequency (can be set between 902-920)
        self.rx_gain = 20  # RX Gain (gain at receiver)
        self.tx_gain = 20  # RFX900 no Tx gain option
        self.usrp_address_source = "addr=192.168.10.2,recv_frame_size=256"
        self.usrp_address_sink = "addr=192.168.10.2,recv_frame_size=256"
        # Each FM0 symbol consists of ADC_RATE/BLF samples (2e6/40e3 = 50 samples)
        # 10 samples per symbol after matched filtering and decimation
        self.num_taps = [1] * 25  # matched to half symbol period
        self.matched_filter = filter.fir_filter_ccc(self.decim, self.num_taps)

        print dir(rfid)

        self.tag_decoder = rfid.tag_decoder(int(self.adc_rate / self.decim))
        self.gate = rfid.gate(int(self.adc_rate / self.decim))
        self.reader = rfid.reader(int(self.adc_rate / self.decim), int(self.dac_rate))
        self.amp = blocks.multiply_const_ff(self.ampl)
        self.to_complex = blocks.float_to_complex()
        self.file_sink_decoder = blocks.file_sink(gr.sizeof_gr_complex * 1, "../misc/data/decoder", False)
        self.u_source()
        self.u_sink()

        # QT
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Top Block")
        # time sink
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass

        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)
        self.settings = Qt.QSettings("GNU Radio", "top_block")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())
        rt = gr.enable_realtime_scheduling()

        self.qtgui_time_sink = qtgui.time_sink_c(
            1024,  # size
            self.adc_rate,  # samp_rate
            "",  # name
            1  # number of inputs
        )
        self.qtgui_time_sink.set_update_time(0.01)
        self.qtgui_time_sink.set_y_axis(0, 10)

        self.qtgui_time_sink.set_y_label('Amplitude', "")

        self.qtgui_time_sink.enable_tags(-1, True)
        self.qtgui_time_sink.set_trigger_mode(qtgui.TRIG_MODE_AUTO, qtgui.TRIG_SLOPE_POS, 1, 0, 0, "")
        self.qtgui_time_sink.enable_autoscale(True)
        self.qtgui_time_sink.enable_grid(True)
        self.qtgui_time_sink.enable_axis_labels(True)
        self.qtgui_time_sink.enable_control_panel(True)

        if not True:
            self.qtgui_time_sink.disable_legend()

        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
                  "magenta", "yellow", "dark red", "dark green", "blue"]
        styles = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
                   -1, -1, -1, -1, -1]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in xrange(2 * 1):
            if len(labels[i]) == 0:
                if (i % 2 == 0):
                    self.qtgui_time_sink.set_line_label(i, "Re{{Data {0}}}".format(i / 2))
                else:
                    self.qtgui_time_sink.set_line_label(i, "Im{{Data {0}}}".format(i / 2))
            else:
                self.qtgui_time_sink.set_line_label(i, labels[i])
            self.qtgui_time_sink.set_line_width(i, widths[i])
            self.qtgui_time_sink.set_line_color(i, colors[i])
            self.qtgui_time_sink.set_line_style(i, styles[i])
            self.qtgui_time_sink.set_line_marker(i, markers[i])
            self.qtgui_time_sink.set_line_alpha(i, alphas[i])

        # consollation
        self._qtgui_time_sink_win = sip.wrapinstance(self.qtgui_time_sink.pyqwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_win)

        self.qtgui_const_sink_x_0 = qtgui.const_sink_c(
            1024,  # size
            "",  # name
            1  # number of inputs
        )
        self.qtgui_const_sink_x_0.set_update_time(0.10)
        self.qtgui_const_sink_x_0.set_y_axis(-2, 2)
        self.qtgui_const_sink_x_0.set_x_axis(-2, 2)
        self.qtgui_const_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, "")
        self.qtgui_const_sink_x_0.enable_autoscale(True)
        self.qtgui_const_sink_x_0.enable_grid(True)
        self.qtgui_const_sink_x_0.enable_axis_labels(True)

        if not True:
            self.qtgui_const_sink_x_0.disable_legend()

        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["blue", "red", "red", "red", "red",
                  "red", "red", "red", "red", "red"]
        styles = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        markers = [0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_const_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_const_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_const_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_const_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_const_sink_x_0.set_line_style(i, styles[i])
            self.qtgui_const_sink_x_0.set_line_marker(i, markers[i])
            self.qtgui_const_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_const_sink_x_0_win = sip.wrapinstance(self.qtgui_const_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_const_sink_x_0_win)

        self.file_sink_source         = blocks.file_sink(gr.sizeof_gr_complex*1, "../misc/data/source", True)
        self.file_sink_matched_filter = blocks.file_sink(gr.sizeof_gr_complex*1, "../misc/data/matched_filter", True)
        self.file_sink_gate           = blocks.file_sink(gr.sizeof_gr_complex*1, "../misc/data/gate", True)
        self.file_sink_decoder        = blocks.file_sink(gr.sizeof_gr_complex*1, "../misc/data/decoder", True)
        self.file_sink_reader         = blocks.file_sink(gr.sizeof_float*1,      "../misc/data/reader", True)
        # CONNECTION
        self.connect((self.gate, 0), (self.qtgui_const_sink_x_0, 0))
        self.connect((self.gate,0), (self.qtgui_time_sink, 0))
        self.connect(self.source, self.matched_filter)
        self.connect(self.matched_filter, self.gate)
        self.connect(self.gate, self.tag_decoder)
        self.connect((self.tag_decoder,0), self.reader)
        self.connect(self.reader, self.amp)
        self.connect(self.amp, self.to_complex)
        self.connect(self.to_complex, self.sink)
        # log
        self.connect(self.source, self.file_sink_source)
        self.connect((self.tag_decoder,1), self.file_sink_decoder) # (Do not comment this line)
        self.connect(self.gate, self.file_sink_gate)
        self.connect(self.matched_filter, self.file_sink_matched_filter)
        def closeEvent(self, event):
            self.settings = Qt.QSettings("GNU Radio", "top_block")
            self.settings.setValue("geometry", self.saveGeometry())
            event.accept()


if __name__ == '__main__':

    from distutils.version import StrictVersion

    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    main_block = reader_top_block()
    main_block.start()
    main_block.show()
    #quit main function
    while (1):
        inp = raw_input("'Q' to quit \n")
        if (inp == "q" or inp == "Q"):
            break
    def quitting():
        main_block.reader.print_results()
        main_block.stop()
        main_block.wait()
    quitting()
    # quit QT
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()

    #LOG
