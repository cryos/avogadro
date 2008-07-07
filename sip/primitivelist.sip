namespace Avogadro {

class PrimitiveList
{
%TypeHeaderCode
#include <avogadro/primitivelist.h>
%End

    public:
      PrimitiveList();
      PrimitiveList( const Avogadro::PrimitiveList &other );
      PrimitiveList( const QList<Avogadro::Primitive *> &other );
      // PrimitiveList &operator=( const Avogadro::PrimitiveList &other );
      // PrimitiveList &operator=(const QList<Avogadro::Primitive *> &other);
      ~PrimitiveList();
      QList<Avogadro::Primitive *> subList( Avogadro::Primitive::Type type ) const;
      QList<Avogadro::Primitive *> list() const;
      bool contains( const Avogadro::Primitive *p ) const;
      void append( Avogadro::Primitive *p );
      void removeAll( Avogadro::Primitive *p );
      int size() const;
      bool isEmpty() const;
      int count( Avogadro::Primitive::Type type ) const;
      int count() const;
      void clear();

};

}; // namespace Avogadro

