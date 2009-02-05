=begin
 *   Copyright 2008 by Richard Dale <richard.j.dale@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
=end

require 'plasma_applet'

module PlasmaScripting
  class Applet < Qt::Object
  slots  "setImmutability(Plasma::ImmutabilityType)",
            :destroy,
            :showConfigurationInterface,
            :raise,
            :lower,
            :flushPendingConstraintsEvents,
            :init

    signals :releaseVisualFocus,
            :geometryChanged,
            :configNeedsSaving,
            :activate

    attr_accessor :applet_script

    def initialize(parent, args = nil)
      super(parent)
      @applet_script = parent
      connect(@applet_script.applet, SIGNAL(:releaseVisualFocus), self, SIGNAL(:releaseVisualFocus))
      connect(@applet_script.applet, SIGNAL(:geometryChanged), self, SIGNAL(:geometryChanged))
      connect(@applet_script.applet, SIGNAL(:configNeedsSaving), self, SIGNAL(:configNeedsSaving))
      connect(@applet_script.applet, SIGNAL(:activate), self, SIGNAL(:activate))
    end

    # If a method is called on a PlasmaScripting::Applet instance is found to be missing
    # then try calling the method on the underlying Plasma::Applet in the ScriptEngine.
    def method_missing(method, *args)
      begin
        super(method, *args)
      rescue
        applet_script.applet.method_missing(method, *args)
      end
    end

    def self.const_missing(name)
      begin
        super(name)
      rescue
        Plasma::Applet.const_missing(name)
      end
    end

    def paintInterface(painter, option, contentsRect)
    end

    def size
      @applet_script.size
    end

    def shape
      @applet_script.shape
    end

    def constraintsEvent(constraints)
    end

    def contextualActions
      return []
    end

    def createConfigurationInterface(dialog)
    end

    def showConfigurationInterface
        dialogId = "#{@applet_script.applet.id}settings#{@applet_script.applet.name}"
        windowTitle = KDE::i18nc("@title:window", "%s Settings" % @applet_script.applet.name)
        @nullManager = KDE::ConfigSkeleton.new(nil)
        @dialog = KDE::ConfigDialog.new(nil, dialogId, @nullManager)
        @dialog.faceType = KDE::PageDialog::Auto
        @dialog.windowTitle = windowTitle
        @dialog.setAttribute(Qt::WA_DeleteOnClose, true)
        createConfigurationInterface(@dialog)
        # TODO: would be nice to not show dialog if there are no pages added?
        # Don't connect to the deleteLater() slot in Ruby as it causes crashes
        # connect(@dialog, SIGNAL(:finished), @nullManager, SLOT(:deleteLater))
        # TODO: Apply button does not correctly work for now, so do not show it
        @dialog.showButton(KDE::Dialog::Apply, false)
        @dialog.show
    end

    def dataEngine(engine)
      @applet_script.dataEngine(engine)
    end

    def package
      @applet_script.package
    end

    def setImmutability(immutabilityType)
      @applet_script.applet.setImmutability(immutabilityType)
    end

    def immutability=(immutabilityType)
      setImmutability(immutabilityType)
    end

    def destroy
      @applet_script.applet.destroy
    end

    def raise
      @applet_script.applet.raise
    end

    def lower
      @applet_script.applet.lower
    end

    def flushPendingConstraintsEvents
      @applet_script.applet.flushPendingConstraintsEvents
    end
  end
end

module PlasmaScriptengineRuby
  class Applet < Plasma::AppletScript
    def initialize(parent, args)
      super(parent)
    end

    def camelize(str)
      str.gsub(/(^|[_-])(.)/) { $2.upcase }
    end

    def init
      oldSize = applet.size

      puts "RubyAppletScript::Applet#init mainScript: #{mainScript}"
      program = Qt::FileInfo.new(mainScript)
      $: << program.path
      load Qt::File.encodeName(program.filePath).to_s
      moduleName = camelize(Qt::Dir.new(package.path).dirName)
      className = camelize(program.baseName)
      puts "RubyAppletScript::Applet#init instantiating: #{moduleName}::#{className}"
      klass = Object.const_get(moduleName.to_sym).const_get(className.to_sym)
      @applet_script = klass.new(self)
      @applet_script.init
      if oldSize.height > 10 && oldSize.width > 10
        applet.resize(oldSize)
      end

      set_up_event_handlers
      return true
    end

    def paintInterface(painter, option, contentsRect)
      @applet_script.paintInterface(painter, option, contentsRect)
    end

    def constraintsEvent(constraints)
      @applet_script.constraintsEvent(constraints)
    end

    def contextualActions
      @applet_script.contextualActions
    end

    def showConfigurationInterface
      @applet_script.showConfigurationInterface
    end

    protected

    def eventFilter(obj, event)
      handler = @event_handlers[event.type.to_i]
      if handler
        @applet_script.send(handler, event)
        return true
      else
        return false
      end
    end

    private

    def set_up_event_handlers
      @event_handlers = {}

      if @applet_script.respond_to?(:mousePressEvent)
        @event_handlers[Qt::Event::GraphicsSceneMousePress.to_i] = :mousePressEvent
      end

      if @applet_script.respond_to?(:contextMenuEvent)
        @event_handlers[Qt::Event::GraphicsSceneContextMenu.to_i] = :contextMenuEvent
      end

      if @applet_script.respond_to?(:dragEnterEvent)
        @event_handlers[Qt::Event::GraphicsSceneDragEnter.to_i] = :dragEnterEvent
      end

      if @applet_script.respond_to?(:dragLeaveEvent)
        @event_handlers[Qt::Event::GraphicsSceneDragLeave.to_i] = :dragLeaveEvent
      end

      if @applet_script.respond_to?(:dragMoveEvent)
        @event_handlers[Qt::Event::GraphicsSceneDragMove.to_i] = :dragMoveEvent
      end

      if @applet_script.respond_to?(:dropEvent)
        @event_handlers[Qt::Event::GraphicsSceneDrop.to_i] = :dropEvent
      end

      if @applet_script.respond_to?(:focusInEvent)
        @event_handlers[Qt::Event::FocusIn.to_i] = :focusInEvent
      end

      if @applet_script.respond_to?(:focusOutEvent)
        @event_handlers[Qt::Event::FocusOut.to_i] = :focusOutEvent
      end

      if @applet_script.respond_to?(:hoverEnterEvent)
        @event_handlers[Qt::Event::GraphicsSceneHoverEnter.to_i] = :hoverEnterEvent
      end

      if @applet_script.respond_to?(:hoverLeaveEvent)
        @event_handlers[Qt::Event::GraphicsSceneHoverLeave.to_i] = :hoverLeaveEvent
      end

      if @applet_script.respond_to?(:hoverMoveEvent)
        @event_handlers[Qt::Event::GraphicsSceneHoverMove.to_i] = :hoverMoveEvent
      end

      if @applet_script.respond_to?(:inputMethodEvent)
        @event_handlers[Qt::Event::InputMethod.to_i] = :inputMethodEvent
      end

      if @applet_script.respond_to?(:keyPressEvent)
        @event_handlers[Qt::Event::KeyPress.to_i] = :keyPressEvent
      end

      if @applet_script.respond_to?(:keyReleaseEvent)
        @event_handlers[Qt::Event::KeyRelease.to_i] = :keyReleaseEvent
      end

      if @applet_script.respond_to?(:mouseDoubleClickEvent)
        @event_handlers[Qt::Event::GraphicsSceneMouseDoubleClick.to_i] = :mouseDoubleClickEvent
      end

      if @applet_script.respond_to?(:mouseMoveEvent)
        @event_handlers[Qt::Event::GraphicsSceneMouseMove.to_i] = :mouseMoveEvent
      end

      if @applet_script.respond_to?(:mousePressEvent)
        @event_handlers[Qt::Event::GraphicsSceneMousePress.to_i] = :mousePressEvent
      end

      if @applet_script.respond_to?(:mouseReleaseEvent)
        @event_handlers[Qt::Event::GraphicsSceneMouseRelease.to_i] = :mouseReleaseEvent
      end

      if @applet_script.respond_to?(:wheelEvent)
        @event_handlers[Qt::Event::GraphicsSceneWheel.to_i] = :wheelEvent
      end

      if !@event_handlers.empty?
        applet.installEventFilter(self)
      end
    end

  end
end

module Plasma
  #
  # Because a PlasmaScript::Applet is not actually a Plasma::Applet we
  # need to 'cheat' in the api, to pretend that it is. So the constructors
  # in the Plasma widget classes will substitute any PlasmaScript::Applet
  # argument passed for the real Plasma::Applet in the ScriptEngine
  #

  class BusyWidget < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class CheckBox < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class ComboBox < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Extender < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class FlashingLabel < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Frame < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class GroupBox < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class IconWidget < Qt::Base
    def initialize(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end
  end

  class Label < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class LineEdit < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Meter < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class PushButton < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class RadioButton < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class ScrollBar < Qt::Base
    def initialize(orientation, parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(orientation, parent.applet_script.applet)
      else
        super
      end
    end
  end

  class SignalPlotter < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class Slider < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class SvgWidget < Qt::Base
    def initialize(*args)
      if args.length > 0 && args[0].kind_of?(PlasmaScripting::Applet)
        args[0] = args[0].applet_script.applet
        super(*args)
      elsif args.length > 2 && args[2].kind_of?(PlasmaScripting::Applet)
        args[2] = args[2].applet_script.applet
        super(*args)
      else
        super
      end
    end
  end

  class TabBar < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class TextEdit < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class ToolTipManager < Qt::Base
    def setContent(widget, data)
      if widget.kind_of?(PlasmaScripting::Applet)
        super(widget.applet_script.applet, data)
      else
        super
      end
    end

    def clearContent(widget)
      if widget.kind_of?(PlasmaScripting::Applet)
        super(widget.applet_script.applet)
      else
        super
      end
    end

    def show(widget)
      if widget.kind_of?(PlasmaScripting::Applet)
        super(widget.applet_script.applet)
      else
        super
      end
    end

    def visible?(widget)
      return isVisible(widget)
    end

    def isVisible(widget)
      if widget.kind_of?(PlasmaScripting::Applet)
        super(widget.applet_script.applet)
      else
        super
      end
    end

    def registerWidget(widget)
      if widget.kind_of?(PlasmaScripting::Applet)
        super(widget.applet_script.applet)
      else
        super
      end
    end

    def unregisterWidget(widget)
      if widget.kind_of?(PlasmaScripting::Applet)
        super(widget.applet_script.applet)
      else
        super
      end
    end
  end

  class TreeView < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

  class WebView < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end
  end

end

module Qt
  class GraphicsProxyWidget < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class QAbstractGraphicsShapeItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsEllipseItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsItemGroup < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsLineItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsPathItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsPixmapItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsPolygonItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsRectItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSimpleTextItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsSvgItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsTextItem < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsWidget < Qt::Base
    def initialize(parent = nil, wFlags = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet, wFlags)
      else
        super
      end
    end

    def parentItem=(item)
      setParentItem(item)
    end

    def setParentItem(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def type(*args)
      method_missing(:type, *args)
    end
  end

  class GraphicsGridLayout < Qt::Base
    def initialize(parent = nil)
      if parent.kind_of?(PlasmaScripting::Applet)
        super(parent.applet_script.applet)
      else
        super
      end
    end

    def addItem(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end

    def alignment(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def setAlignment(item, alignment)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet, alignment)
      else
        super
      end
    end
  end

  class GraphicsLinearLayout < Qt::Base
    def initialize(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end

    def addItem(*args)
      sargs = []
      for i in 0...args.length do
        if args[i].kind_of?(PlasmaScripting::Applet)
          sargs << args[i].applet_script.applet
        else
          sargs << args[i]
        end
      end
      super(*sargs)
    end

    def alignment(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end

    def insertItem(index, item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(index, item.applet_script.applet)
      else
        super
      end
    end

    def setAlignment(item, alignment)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet, alignment)
      else
        super
      end
    end

    def setStretchFactor(item, stretch)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet, stretch)
      else
        super
      end
    end

    def stretchFactor(item)
      if item.kind_of?(PlasmaScripting::Applet)
        super(item.applet_script.applet)
      else
        super
      end
    end
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
